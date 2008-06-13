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
 * - implement contact edit
 * - add support to show deleted contacts using 'showdeleted' query parameter
 * - support get/add/edit contact's photos
 * - for some firewalls, X-HTTP-Method-Override: DELETE can be required
 *
 */
#include <string.h>
#include "internal_gcal.h"
#include "gcontact.h"
#include "gcal_parser.h"


struct gcal_contact *gcal_get_contacts(struct gcal_resource *ptr_gcal,
				       size_t *length)

{
	int result = -1;
	struct gcal_contact *ptr_res = NULL;

	if (!ptr_gcal)
		goto exit;

	if (!ptr_gcal->buffer || !ptr_gcal->has_xml)
		goto exit;

	ptr_gcal->document = build_dom_document(ptr_gcal->buffer);
	if (!ptr_gcal->document)
		goto exit;


	result = get_entries_number(ptr_gcal->document);
	if (result == -1)
		goto cleanup;

	ptr_res = malloc(sizeof(struct gcal_contact) * result);
	if (!ptr_res)
		goto cleanup;

	*length = result;
	result = extract_all_contacts(ptr_gcal->document, ptr_res, result);
	if (result == -1) {
		free(ptr_res);
		ptr_res = NULL;
	}

	goto exit;

cleanup:
	clean_dom_document(ptr_gcal->document);
	ptr_gcal->document = NULL;

exit:

	return ptr_res;

}

static void clean_string(char *ptr_str)
{
	if (ptr_str)
		free(ptr_str);
}

void gcal_destroy_contact(struct gcal_contact *contact)
{
	if (!contact)
		return;

	clean_string(contact->id);
	clean_string(contact->updated);
	clean_string(contact->title);
	clean_string(contact->edit_uri);
	clean_string(contact->email);

	/* Extra fields */
	clean_string(contact->content);
	clean_string(contact->org_name);
	clean_string(contact->org_title);
	clean_string(contact->im);
	clean_string(contact->phone_number);
	clean_string(contact->post_address);

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

int gcal_create_contact(struct gcal_resource *ptr_gcal,
			struct gcal_contact *contact,
			struct gcal_contact *updated)
{
	int result = -1, length;
	char *xml_contact = NULL, *buffer;

	if ((!contact) || (!ptr_gcal))
		return result;

	result = xmlcontact_create(contact, &xml_contact, &length);
	if (result == -1)
		goto exit;

	/* Mounts URL */
	length = sizeof(GCONTACT_START) + sizeof(GCONTACT_END) +
		strlen(ptr_gcal->user) + 1;
	buffer = (char *) malloc(length);
	if (!buffer)
		goto cleanup;
	snprintf(buffer, length - 1, "%s%s%s", GCONTACT_START,
		 ptr_gcal->user, GCONTACT_END);

	result = up_entry(xml_contact, ptr_gcal, buffer, POST,
			  GCAL_EDIT_ANSWER);

	/* Parse buffer and create the new contact object */
	if (!updated)
		goto cleanup;
	result = -2;
	ptr_gcal->document = build_dom_document(ptr_gcal->buffer);
	if (!ptr_gcal->document)
		goto cleanup;

	/* There is only one 'entry' in the buffer */
	result = extract_all_contacts(ptr_gcal->document, updated, 1);
	if (result == -1)
		goto xmlclean;

	result = 0;


xmlclean:
	clean_dom_document(ptr_gcal->document);
	ptr_gcal->document = NULL;

cleanup:
	if (xml_contact)
		free(xml_contact);
	if (buffer)
		free(buffer);

exit:
	return result;

}

int gcal_delete_contact(struct gcal_resource *ptr_gcal,
			struct gcal_contact *contact)
{
	int result = -1, length;
	char *h_auth;

	if (!contact || !ptr_gcal)
		goto exit;

	/* Must cleanup HTTP buffer between requests */
	clean_buffer(ptr_gcal);

	/* TODO: add X-HTTP header */
	length = strlen(ptr_gcal->auth) + sizeof(HEADER_GET) + 1;
	h_auth = (char *) malloc(length);
	if (!h_auth)
		goto exit;
	snprintf(h_auth, length - 1, "%s%s", HEADER_GET, ptr_gcal->auth);

	curl_easy_setopt(ptr_gcal->curl, CURLOPT_CUSTOMREQUEST, "DELETE");
	result = http_post(ptr_gcal, contact->edit_uri,
			   "Content-Type: application/atom+xml",
			   NULL,
			   h_auth,
			   NULL, GCAL_DEFAULT_ANSWER);

	if (h_auth)
		free(h_auth);

exit:

	return result;
}

int gcal_edit_contact(struct gcal_resource *ptr_gcal,
		      struct gcal_contact *contact,
		      struct gcal_contact *updated)
{

	int result = -1, length;
	char *xml_contact = NULL;

	if ((!contact) || (!ptr_gcal))
		goto exit;

	result = xmlcontact_create(contact, &xml_contact, &length);
	if (result == -1)
		goto exit;

	result = up_entry(xml_contact, ptr_gcal, contact->edit_uri, PUT,
			  GCAL_DEFAULT_ANSWER);

	/* Parse buffer and create the new contact object */
	if (!updated)
		goto cleanup;
	result = -2;
	ptr_gcal->document = build_dom_document(ptr_gcal->buffer);
	if (!ptr_gcal->document)
		goto cleanup;

	/* There is only one 'entry' in the buffer */
	result = extract_all_contacts(ptr_gcal->document, updated, 1);
	if (result == -1)
		goto xmlclean;

	result = 0;


xmlclean:
	clean_dom_document(ptr_gcal->document);
	ptr_gcal->document = NULL;

cleanup:

	if (xml_contact)
		free(xml_contact);

exit:
	return result;

}