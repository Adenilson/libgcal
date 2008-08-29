/*
 * @file   utest_userapi.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Started on June 24 2008
 *
 * @brief  Implementation module for user api unit tests.
 *
 * This is a good place to look when learning to use libgcal. The following
 * operations are covered:
 * - authentication
 * - getting all calendar events
 * - accessing then
 * - adding a new calendar event
 * - editing and deleting an event
 * - querying for updated calendar events
 *
 */

#include "utest_userapi.h"
#include "gcalendar.h"
#include "gcontact.h"
#include <stdio.h>
#include <string.h>

/* I use this variable to exchange data between the contacts tests */
char *deleted_contact_id = NULL;

START_TEST (test_get_calendar)
{
	gcal_t gcal;
	struct gcal_event_array event_array;
	int result;

	gcal = gcal_new(GCALENDAR);
	fail_if(gcal == NULL, "Failed constructing gcal object!");

	result = gcal_get_authentication(gcal, "gcal4tester", "66libgcal");
	fail_if(result == -1, "Cannot authenticate!");

	result = gcal_get_events(gcal, &event_array);
	fail_if(result == -1, "Failed downloading events!");
	fail_if(event_array.length < 1, "gcal4tester must have at least"
		"1 event!");

	/* Cleanup */
	gcal_cleanup_events(&event_array);
	gcal_delete(gcal);

}
END_TEST


START_TEST (test_access_calendar)
{
	gcal_t gcal;
	struct gcal_event_array event_array;
	gcal_event event;
	size_t i;
	int result;
	char *ptr;

	gcal = gcal_new(GCALENDAR);
	result = gcal_get_authentication(gcal, "gcal4tester", "66libgcal");
	result = gcal_get_events(gcal, &event_array);

	/* Access events properties */
	for (i = 0; i < event_array.length; ++i) {

		/* Access i-nth calendar event */
		event = gcal_event_element(&event_array, i);

		/* Common fields between calendar and contacts are
		 * of type 'gcal_entry'
		 */
		ptr = gcal_event_get_id(event);
		ptr = gcal_event_get_updated(event);
		ptr = gcal_event_get_title(event);
		ptr = gcal_event_get_url(event);

		fail_if(ptr == NULL, "Can't get edit url!");

		/* This are the fields unique to calendar events */
		ptr = gcal_event_get_content(event);
		ptr = gcal_event_get_recurrent(event);
		ptr = gcal_event_get_start(event);
		ptr = gcal_event_get_end(event);
		ptr = gcal_event_get_where(event);
		ptr = gcal_event_get_status(event);
	}

	/* This code block is for testing overflow only! Please dont use
	 * gcal in this way.
	 */
	ptr = gcal_event_get_id(gcal_event_element(&event_array,
						      event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_updated(gcal_event_element(&event_array,
							   event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_title(gcal_event_element(&event_array,
							 event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_url(gcal_event_element(&event_array,
						       event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_content(gcal_event_element(&event_array,
							   event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_recurrent(gcal_event_element(&event_array,
							     event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_start(gcal_event_element(&event_array,
							 event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_end(gcal_event_element(&event_array,
						       event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_where(gcal_event_element(&event_array,
							 event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_status(gcal_event_element(&event_array,
							  event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");


	/* Cleanup */
	gcal_cleanup_events(&event_array);
	gcal_delete(gcal);
}
END_TEST


START_TEST (test_oper_event_event)
{
	gcal_t gcal;
	gcal_event event;
	int result;

	/* Create a new event object */
	event = gcal_event_new();
	fail_if (!event, "Cannot construct event object!");
	gcal_event_set_title(event, "A new event");
	gcal_event_set_content(event, "Here goes the description");
	gcal_event_set_start(event, "2008-06-24T16:00:00Z");
	gcal_event_set_end(event, "2008-06-24T18:00:00Z");
	gcal_event_set_where(event, "A nice place for a meeting");

	/* Create a gcal object and authenticate */
	gcal = gcal_new(GCALENDAR);
	result = gcal_get_authentication(gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Failed getting authentication");

	/* Add a new event */
	result = gcal_add_event(gcal, event);
	fail_if(result == -1, "Failed adding a new event!");


	/* Edit this event */
	gcal_event_set_title(event, "Changing the title");
	result = gcal_update_event(gcal, event);
	fail_if(result == -1, "Failed editing event!");

	/* Delete this event (note: google doesn't really deletes
	 * the event, but set its status to 'cancelled' and keeps
	 * then for nearly 4 weeks).
	 */
	result = gcal_erase_event(gcal, event);
	fail_if(result == -1, "Failed deleting event!");

	/* Cleanup */
	gcal_event_delete(event);
	gcal_delete(gcal);
}
END_TEST

START_TEST (test_query_event_updated)
{
	gcal_t gcal;
	struct gcal_event_array event_array;
	gcal_event event;
	int result;
	/* Previous test added/edited/deleted an event with this title */
	char *title = "Changing the title";

	gcal = gcal_new(GCALENDAR);
	result = gcal_get_authentication(gcal, "gcalntester", "77libgcal");

	/* This will query for all updated events (fall in this category
	 * added/deleted/updated events) starting for 06:00Z UTC of today).
	 */
	result = gcal_get_updated_events(gcal, &event_array, NULL);
	fail_if(result == -1, "Failed downloading updated events!");
	fail_if(event_array.length < 1, "If previous test was ok, it must"
		" return at least one updated event!");

	/* Google returns the last updated event first */
	event = gcal_event_element(&event_array, 0);
	if (gcal_event_is_deleted(event))
	    if (gcal_event_get_title(event))
		    result = strcmp(gcal_event_get_title(event), title);
	else
		result = -1;
	fail_if(result != 0, "Cannot locate event!");

	/* Cleanup */
	gcal_cleanup_events(&event_array);
	gcal_delete(gcal);

}
END_TEST

START_TEST (test_get_contacts)
{
	gcal_t gcal;
	struct gcal_contact_array contact_array;
	int result;

	gcal = gcal_new(GCONTACT);
	fail_if(gcal == NULL, "Failed constructing gcal object!");

	result = gcal_get_authentication(gcal, "gcal4tester", "66libgcal");
	fail_if(result == -1, "Cannot authenticate!");

	result = gcal_get_contacts(gcal, &contact_array);
	fail_if(result == -1, "Failed downloading contacts!");
	fail_if(contact_array.length != 2, "gcal4tester must have only"
		"2 contacts!");

	/* Cleanup */
	gcal_cleanup_contacts(&contact_array);
	gcal_delete(gcal);

}
END_TEST


START_TEST (test_access_contacts)
{
	gcal_t gcal;
	struct gcal_contact_array contact_array;
	gcal_contact contact;
	size_t i;
	int result;
	char *ptr;

	gcal = gcal_new(GCONTACT);
	result = gcal_get_authentication(gcal, "gcal4tester", "66libgcal");
	result = gcal_get_contacts(gcal, &contact_array);

	/* Access events properties */
	for (i = 0; i < contact_array.length; ++i) {

		/* Access i-nth calendar event */
		contact = gcal_contact_element(&contact_array, i);

		/* Common fields between calendar and contacts are
		 * of type 'gcal_entry'
		 */
		ptr = gcal_contact_get_id(contact);
		ptr = gcal_contact_get_updated(contact);
		/* Tip: it *is* valid a contact have no name. */
		ptr = gcal_contact_get_title(contact);
		ptr = gcal_contact_get_url(contact);

		fail_if(ptr == NULL, "Can't get edit url!");

		/* This are the fields unique to calendar events */
		ptr = gcal_contact_get_email(contact);
		ptr = gcal_contact_get_content(contact);
		ptr = gcal_contact_get_orgname(contact);
		ptr = gcal_contact_get_orgtitle(contact);
		ptr = gcal_contact_get_im(contact);
		ptr = gcal_contact_get_phone(contact);
		ptr = gcal_contact_get_address(contact);

	}

	/* This code block is for testing overflow only! Please dont use
	 * gcal in this way.
	 */
	ptr = gcal_contact_get_id(gcal_contact_element(&contact_array,
						   contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_updated(gcal_contact_element(&contact_array,
							contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_title(gcal_contact_element(&contact_array,
						      contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_url(gcal_contact_element(&contact_array,
						    contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_email(gcal_contact_element(&contact_array,
							contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_content(gcal_contact_element(&contact_array,
							contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_orgname(gcal_contact_element(&contact_array,
							    contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_orgtitle(gcal_contact_element(&contact_array,
						      contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_im(gcal_contact_element(&contact_array,
						      contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_phone(gcal_contact_element(&contact_array,
							  contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_address(gcal_contact_element(&contact_array,
						      contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");


	/* Cleanup */
	gcal_cleanup_contacts(&contact_array);
	gcal_delete(gcal);

}
END_TEST

START_TEST (test_oper_contact)
{
	gcal_t gcal;
	gcal_contact contact;
	int result;

	/* Create a new contact object */
	contact = gcal_contact_new();
	fail_if (!contact, "Cannot construct contact object!");
	gcal_contact_set_title(contact, "John Doe");
	gcal_contact_set_email(contact, "john.doe@foo.bar.com");

	/* Create a gcal object and authenticate */
	gcal = gcal_new(GCONTACT);
	result = gcal_get_authentication(gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Failed getting authentication");

	/* Add a new contact */
	result = gcal_add_contact(gcal, contact);
	fail_if(result == -1, "Failed adding a new contact!");

	/* Edit this contact */
	gcal_contact_set_title(contact, "John 'The Generic' Doe");
	fail_if(result == -1, "Failed editing contact!");
	gcal_contact_set_email(contact, "john.super.doe@foo.bar.com");
	fail_if(result == -1, "Failed editing contact!");
	result = gcal_update_contact(gcal, contact);
	fail_if(result == -1, "Failed uploading edited contact!");

	/* Save this contact's ID to use it in the next test, where we
	 * search for updated contacts.
	 */
	deleted_contact_id = strdup(gcal_contact_get_id(contact));

	/* Delete this contact (note: google still keeps a deleted contact
	 * for nearly 4 weeks. Its possible to retrieve it using
	 * 'gcal_deleted(gcal, SHOW)' before downloading contacts)
	 */
	result = gcal_erase_contact(gcal, contact);
	fail_if(result == -1, "Failed deleting contact!");

	/* Cleanup */
	gcal_contact_delete(contact);
	gcal_delete(gcal);
}
END_TEST

START_TEST (test_query_contact_updated)
{
	gcal_t gcal;
	struct gcal_contact_array contact_array;
	gcal_contact contact;
	int result;
	size_t tmp;

	gcal = gcal_new(GCONTACT);
	result = gcal_get_authentication(gcal, "gcalntester", "77libgcal");

	/* This will query for all updated contacts (fall in this category
	 * added/updated contacts) starting for 06:00Z UTC of today).
	 */
	result = gcal_get_updated_contacts(gcal, &contact_array, NULL);
	fail_if(result == -1, "Failed downloading updated contacts!");
	fail_if(contact_array.length > 3, "This user should not have more"
		" than 3 updated contacts!");

	/* Now we query for deleted contacts (previous test
	 * added/updated/deleted one contact, remember?)
	 */
	tmp = contact_array.length;
	gcal_deleted(gcal, SHOW);
	result = gcal_get_updated_contacts(gcal, &contact_array, NULL);
	fail_if(result == -1, "Failed downloading updated contacts!");
	fail_if(contact_array.length <= tmp , "If previous test was ok, it must"
		" return one more contact!");

	/* FIXME: Contacts doesn't return the last updated contact
	 * first when running with 'showdeleted'.
	 */
	result = -1;
 	for (tmp = 0; tmp < contact_array.length; ++tmp) {
		contact = gcal_contact_element(&contact_array, tmp);
		/* only compare deleted contacts */
		if (gcal_contact_is_deleted(contact))
			result = strcmp(gcal_contact_get_id(contact),
					deleted_contact_id);
		if (!result)
			break;
	}

	fail_if(result != 0, "Cannot locate contact!");

	/* Cleanup */
	gcal_cleanup_contacts(&contact_array);
	gcal_delete(gcal);

}
END_TEST


TCase *gcal_userapi(void)
{
	TCase *tc = NULL;
	int timeout_seconds = 50;
	tc = tcase_create("gcaluserapi");
	tcase_set_timeout (tc, timeout_seconds);

	tcase_add_test(tc, test_get_calendar);
	tcase_add_test(tc, test_access_calendar);
	tcase_add_test(tc, test_oper_event_event);
	tcase_add_test(tc, test_query_event_updated);
	tcase_add_test(tc, test_get_contacts);
	tcase_add_test(tc, test_access_contacts);
	tcase_add_test(tc, test_oper_contact);
	tcase_add_test(tc, test_query_contact_updated);
	return tc;
}
