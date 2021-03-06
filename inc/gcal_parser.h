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
#ifndef __GCAL_PARSER__
#define __GCAL_PARSER__
/**
 * @file   gcal_parser.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Mon Mar 31 11:17:02 2008
 *
 * @brief  A thin layer over \ref atom_parser.h, so I can plug another
 * XML parser to libgcal if required.
 * It creates a DOM document from libgcal atom stream and provides functions
 * wrappers to extract data.
 */

#include <libxml/parser.h>
#include "internal_gcal.h"
#include "gcal.h"
#include "gcontact.h"

/** Parses the returned HTML page and extracts the redirection URL
 * that has the Atom feed.
 *
 * \todo Save the xmlDoc structure for future reuse (maybe within
 * structure \ref gcal_resource).
 *
 * @param data Raw data (the HTML page).
 * @param length Data buffer length.
 * @param url Pointer to the pointer (ouch!) that will receive the URL
 * (you should cleanup its memory). It will point to NULL if there is
 * not a URL in the raw data buffer.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int get_the_url(char *data, int length, char **url);

/** Parses an entry XML (being calendar or contact) and extracts the edit
 * URL field (required for edit/delete operations).
 *
 * @param data Raw XML (an entry).
 * @param length Data buffer lenght.
 * @param url Pointer to the pointer (ouch!) that will receive the edit URL
 * (you should cleanup its memory). It will point to NULL if there is
 * not a URL in the raw XML buffer.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int get_edit_url(char *data, int length, char **url);

/** Parses an entry XML (being calendar or contacts) and extracts the ETag
 * (gd:etag) attribute required to do some operations using Google
 * Data API 2.0.
 *
 * @param data Raw XML (an entry).
 * @param length Data buffer lenght.
 * @param url Pointer to the pointer (ouch!) that will receive the ETag
 * (you should cleanup its memory). It will point to NULL if there is
 * not an etag attribute in the raw XML buffer.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int get_edit_etag(char *data, int length, char **url);

/** Builds a DOM tree from a XML string.
 *
 * This is a thin wrapper to \ref build_doc_tree.
 *
 * @param xml_data A pointer to a string with XML content.
 *
 * @return NULL on error, a pointer to a document in sucess.
 */
dom_document *build_dom_document(char *xml_data);


/** Clean up a DOM tree.
 *
 * This is a thin wrapper to \ref clean_doc_tree.
 * @param doc A pointer to a document data type.
 */
void clean_dom_document(dom_document *doc);

/** Return the number of calendar entries in the document.
 *
 * This is a thin wrapper to \ref clean_doc_tree.
 * @param doc A pointer to a document data type.
 *
 * @return -1 on error or the number of entries.
 */
int get_entries_number(dom_document *doc);


/** Return the number of calendars in the document.
 *
 * This is a thin wrapper to \ref clean_doc_tree.
 * @param doc A pointer to a document data type.
 *
 * @return -1 on error or the number of calendars.
 */
int get_entries_number_xml(dom_document *doc);


/** Retrieve a DOM document of the Atom stream, it will extract all
 *  the calendars and parse them, storing specified indexed entry within
 *  a \ref gcal_resource.
 *
 * @param doc A document pointer to the Atom stream.
 * @param index the calendar index item to fetch
 * @param res A pointer to a pre-allocated \erf gcal_resource pointer.
 *
 * @return 0 on success, -1 on error.
 */
int get_calendar_entry(dom_document *doc, int index, struct gcal_resource *res);


/** Receiving a DOM document of the Atom stream, it will extract all the event
 * entries and parse them, storing each entry field in a vector of
 * \ref gcal_event.
 *
 * It depends on \ref atom_extract_data and \ref atom_get_entries.
 *
 * @param doc A document pointer to the Atom stream.
 *
 * @param data_extract A pointer to a pre-allocated vector \ref gcal_event.
 *
 * @param length Its length, should be the same as the number of entries. See
 * also \ref get_entries_number.
 *
 * @return 0 on success, -1 on error.
 */
int extract_all_entries(dom_document *doc,
			struct gcal_event *data_extract, int length);


/** Creates the XML for a new calendar entry.
 *
 * It depends on \ref xmlentry_init_resources and
 * \ref xmlentry_destroy_resources.
 *
 * @param entry A pointer to an calendar entry event (see \ref gcal_event).
 *
 * @param xml_entry Pointer to pointer string (you must free its memory!).
 *
 * @param length A pointer to a variable that will have its length.
 *
 * @return 0 on sucess, -1 on error.
 */
int xmlentry_create(struct gcal_event *entry, char **xml_entry, int *length);


/** Receiving a DOM document of the Atom stream, it will extract all the
 * contacts and parse then, storing each contact in a vector of
 * \ref gcal_contact.
 *
 * It depends on \ref atom_extract_data and \ref atom_get_entries.
 *
 * @param doc A document pointer with the Atom stream.
 *
 * @param data_extract A pointer to a pre-allocated vector \ref gcal_contact.
 *
 * @param length Its length, should be the same as the number of entries. See
 * also \ref get_entries_number.
 *
 * @return 0 on success, -1 on error.
 */
int extract_all_contacts(dom_document *doc,
			 struct gcal_contact *data_extract, int length);


/** Creates the XML for a new contact entry.
 *
 * It depends on \ref xmlentry_init_resources and
 * \ref xmlentry_destroy_resources.
 *
 * @param contact A pointer to a contact (see \ref gcal_contact).
 *
 * @param xml_contact Pointer to pointer string (you must free its memory!).
 *
 * @param length A pointer to a variable that will have its length.
 *
 * @return 0 on sucess, -1 on error.
 */
int xmlcontact_create(struct gcal_contact *contact, char **xml_contact,
		      int *length);

#endif
