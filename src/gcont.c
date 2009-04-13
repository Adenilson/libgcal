/*
Copyright (c) 2008 Instituto Nokia de Tecnologia
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the INdT nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * @file   gcontact.c
 * @author Adenilson Cavalcanti
 * @date   Fri May 30 15:30:35 2008
 *
 * @brief  Base file for google contacts service access library.
 *
 * \todo:
 * - for some firewalls, X-HTTP-Method-Override: DELETE can be required
 * - support contact's photo download (upload is working ok!)
 *
 */
#include <string.h>
#include "internal_gcal.h"
#include "gcontact.h"
#include "gcal_parser.h"


static size_t write_cb_binary(void *ptr, size_t count, size_t chunk_size,
			      void *data)
{

	size_t size = count * chunk_size;
	struct gcal_resource *gcal_ptr = (struct gcal_resource *)data;
	char *ptr_tmp;

	if (size > (gcal_ptr->length - gcal_ptr->previous_length - 1)) {
		gcal_ptr->length = gcal_ptr->length + size + 1;
		ptr_tmp = realloc(gcal_ptr->buffer, gcal_ptr->length);

		if (!ptr_tmp) {
			if (gcal_ptr->fout_log)
				fprintf(gcal_ptr->fout_log,
					"write_cb: Failed relloc!\n");
			goto exit;
		}

		gcal_ptr->buffer = ptr_tmp;
	}

	memcpy(gcal_ptr->buffer + gcal_ptr->previous_length, ptr, size);
	gcal_ptr->previous_length += size;

exit:
	return size;
}

struct gcal_contact *gcal_get_all_contacts(struct gcal_resource *gcalobj,
					   size_t *length)

{
	int result = -1;
	size_t i = 0;
	struct gcal_contact *ptr_res = NULL;

	if (!gcalobj)
		goto exit;

	if (!gcalobj->buffer || !gcalobj->has_xml)
		goto exit;

	gcalobj->document = build_dom_document(gcalobj->buffer);
	if (!gcalobj->document)
		goto exit;


	result = get_entries_number(gcalobj->document);
	if (result == -1)
		goto cleanup;

	ptr_res = malloc(sizeof(struct gcal_contact) * result);
	if (!ptr_res)
		goto cleanup;
	memset(ptr_res, 0, sizeof(struct gcal_contact) * result);

	*length = result;
	for (i = 0; i < *length; ++i) {
		gcal_init_contact((ptr_res + i));
		if (gcalobj->store_xml_entry)
			(ptr_res + i)->common.store_xml = 1;
	}

	result = extract_all_contacts(gcalobj->document, ptr_res, *length);
	if (result == -1) {
		free(ptr_res);
		ptr_res = NULL;
	}

	/* Check contacts with photo and download the pictures */
	for (i = 0; i < *length; ++i)
		if (ptr_res[i].photo_length) {
			if (gcalobj->fout_log)
				fprintf(gcalobj->fout_log,
					"contact with photo!\n");

 			result = get_follow_redirection(gcalobj,
 							ptr_res[i].photo,
							write_cb_binary);
			ptr_res[i].photo_data = malloc(sizeof(char) *
						       gcalobj->length);
			if (!ptr_res[i].photo_data)
				goto exit;
			ptr_res[i].photo_length = gcalobj->length;
			memcpy(ptr_res[i].photo_data, gcalobj->buffer,
			       ptr_res[i].photo_length);

			clean_buffer(gcalobj);

		} else if (gcalobj->fout_log)
			fprintf(gcalobj->fout_log, "contact without photo!\n");

	goto exit;

cleanup:
	clean_dom_document(gcalobj->document);
	gcalobj->document = NULL;

exit:

	return ptr_res;
}

static void clean_string(char *ptr_str)
{
	if (ptr_str)
		free(ptr_str);
}

void gcal_init_contact(struct gcal_contact *contact)
{
	if (!contact)
		return;

	contact->common.store_xml = 0;
	contact->common.id = contact->common.updated = NULL;
	contact->common.title = contact->common.xml = NULL;
	contact->common.edit_uri = contact->common.etag = NULL;
	contact->email = contact->content = NULL;
	contact->org_name = contact->org_title = contact->im = NULL;
	contact->phone_number = contact->post_address = NULL;
	contact->photo = contact->photo_data = NULL;
	contact->photo_length = 0;
}

void gcal_destroy_contact(struct gcal_contact *contact)
{
	if (!contact)
		return;

	clean_string(contact->common.id);
	clean_string(contact->common.updated);
	clean_string(contact->common.title);
	clean_string(contact->common.edit_uri);
	clean_string(contact->common.etag);
	clean_string(contact->email);
	clean_string(contact->common.xml);

	/* Extra fields */
	clean_string(contact->content);
	clean_string(contact->org_name);
	clean_string(contact->org_title);
	clean_string(contact->im);
	clean_string(contact->phone_number);
	clean_string(contact->post_address);
	clean_string(contact->photo);
	clean_string(contact->photo_data);
	contact->photo_length = 0;
}

void gcal_destroy_contacts(struct gcal_contact *contacts, size_t length)
{

	size_t i = 0;
	if (!contacts)
		return;

	for (; i < length; ++i)
		gcal_destroy_contact((contacts + i));

	free(contacts);
}

int gcal_create_contact(struct gcal_resource *gcalobj,
			struct gcal_contact *contact,
			struct gcal_contact *updated)
{
	int result = -1, length;
	char *xml_contact = NULL, *buffer;

	if ((!contact) || (!gcalobj))
		return result;

	result = xmlcontact_create(contact, &xml_contact, &length);
	if (result == -1)
		goto exit;

	/* Mounts URL */
	length = sizeof(GCONTACT_START) + sizeof(GCONTACT_END) +
		strlen(gcalobj->user) + 1;
	buffer = (char *) malloc(length);
	if (!buffer)
		goto cleanup;
	snprintf(buffer, length - 1, "%s%s%s", GCONTACT_START,
		 gcalobj->user, GCONTACT_END);

	result = up_entry(xml_contact, strlen(xml_contact), gcalobj,
			  buffer, NULL, POST, NULL, GCAL_EDIT_ANSWER);
	if (result)
		goto cleanup;

	/* Copy raw XML */
	if (gcalobj->store_xml_entry) {
		if (contact->common.xml)
			free(contact->common.xml);
		if (!(contact->common.xml = strdup(gcalobj->buffer)))
			goto cleanup;
	}

	/* Parse buffer and create the new contact object */
	if (!updated)
		goto cleanup;
	result = -2;
	gcalobj->document = build_dom_document(gcalobj->buffer);
	if (!gcalobj->document)
		goto cleanup;

	/* There is only one 'entry' in the buffer */
	gcal_init_contact(updated);
	result = extract_all_contacts(gcalobj->document, updated, 1);
	if (result == -1)
		goto xmlclean;

	/* Adding photo is the same as an edit operation */
	if (contact->photo_length) {
		result = up_entry(contact->photo_data, contact->photo_length,
				  gcalobj, updated->photo, NULL,
				  PUT, "Content-Type: image/*",
				  GCAL_DEFAULT_ANSWER);
		if (result)
			goto cleanup;

	}

	result = 0;


xmlclean:
	clean_dom_document(gcalobj->document);
	gcalobj->document = NULL;

cleanup:
	if (xml_contact)
		free(xml_contact);
	if (buffer)
		free(buffer);

exit:
	return result;

}

int gcal_delete_contact(struct gcal_resource *gcalobj,
			struct gcal_contact *contact)
{
	int result = -1, length;
	char *h_auth;

	if (!contact || !gcalobj)
		goto exit;

	/* Must cleanup HTTP buffer between requests */
	clean_buffer(gcalobj);

	/* TODO: add X-HTTP header */
	length = strlen(gcalobj->auth) + sizeof(HEADER_GET) + 1;
	h_auth = (char *) malloc(length);
	if (!h_auth)
		goto exit;
	snprintf(h_auth, length - 1, "%s%s", HEADER_GET, gcalobj->auth);

	curl_easy_setopt(gcalobj->curl, CURLOPT_CUSTOMREQUEST, "DELETE");
	result = http_post(gcalobj, contact->common.edit_uri,
			   "Content-Type: application/atom+xml",
			   /* Google Data API 2.0 requires ETag */
			   "If-Match: *",
			   h_auth,
			   NULL, NULL, 0, GCAL_DEFAULT_ANSWER);

	/* Restores curl context to previous standard mode */
	curl_easy_setopt(gcalobj->curl, CURLOPT_CUSTOMREQUEST, NULL);

	if (h_auth)
		free(h_auth);

exit:

	return result;
}

int gcal_edit_contact(struct gcal_resource *gcalobj,
		      struct gcal_contact *contact,
		      struct gcal_contact *updated)
{

	int result = -1, length;
	char *xml_contact = NULL;

	if ((!contact) || (!gcalobj))
		goto exit;

	result = xmlcontact_create(contact, &xml_contact, &length);
	if (result == -1)
		goto exit;

	result = up_entry(xml_contact, strlen(xml_contact), gcalobj,
			  contact->common.edit_uri,
			  /* Google Data API 2.0 requires ETag */
			  "If-Match: *",
			  PUT, NULL, GCAL_DEFAULT_ANSWER);
	if (result)
		goto cleanup;

	/* Copy raw XML */
	if (gcalobj->store_xml_entry) {
		if (contact->common.xml)
			free(contact->common.xml);
		if (!(contact->common.xml = strdup(gcalobj->buffer)))
			goto cleanup;
	}

	/* Parse buffer and create the new contact object */
	if (!updated)
		goto cleanup;
	result = -2;
	gcalobj->document = build_dom_document(gcalobj->buffer);
	if (!gcalobj->document)
		goto cleanup;

	/* There is only one 'entry' in the buffer */
	gcal_init_contact(updated);
	result = extract_all_contacts(gcalobj->document, updated, 1);
	if (result == -1)
		goto xmlclean;

	/* Adding photo is the same as an edit operation */
	if (contact->photo_length) {
		result = up_entry(contact->photo_data, contact->photo_length,
				  gcalobj, updated->photo,
				  /* Google Data API 2.0 requires ETag */
				  "If-Match: *",
				  PUT, "Content-Type: image/*",
				  GCAL_DEFAULT_ANSWER);
		if (result)
			goto cleanup;

	}

	result = 0;


xmlclean:
	clean_dom_document(gcalobj->document);
	gcalobj->document = NULL;

cleanup:

	if (xml_contact)
		free(xml_contact);

exit:
	return result;

}
