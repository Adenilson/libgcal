## Description ##
ATTENTION: I no longer have time to maintain libgcal and thanks to changes on google services to use OAuth 2.0 for login ([client login is deprecated](http://googledevelopers.blogspot.com/2012/04/changes-to-deprecation-policies-and-api.html)) plus Google plans to [shutdown](https://developers.google.com/google-apps/calendar/v2/developers_guide_protocol) Calendar API v2 in November 2014 it doesn't make much sense to continue development. I'm keeping the project page online for historical purposes. As at the time of writing (2014-10-01), google doesn't support a C/C++ client library (and probably never will).

Its an ANSI C library that does allow communication with google calendar and contacts, useful if you need to support this in a C or C++ application.

Currently, libgcal implements:

```
 - authentication
 - get all events/contacts
 - atom stream parsing
 - access to individual events/contacts
 - add/delete/edit using gcal objects
 - add/delete/edit using raw XML
 - query for updated events/contacts
 - use of http proxy
 - timezone
 - contact photo download/upload
 - google data protocol GCalendar 2.0 and Contacts 3.0
 - can be used with a C++ application
 - google hosted accounts (i.e. joe@foobar.com)
```

What is missing: batch commit. Master has code for creation of recurrent events, support other calendars (besides main calendar).

Currently libgcal is tested on Linux and Mac OSX Leopard, but should be portable to all platforms supported by its dependencies.

Dependencies: libxml, libcurl, ANSI C (ca-certificates is an indirect dependency)

## Examples ##
Bellow you can see some examples of libgcal usage (they could have less lines, but checking for function return values is **always** a good programming practice).

**Get all contacts**
```
#include <stdio.h>
#include <gcalendar.h>
#include <gcontact.h>

int main(int argc, char *argv[])
{
        gcal_t gcal;
        gcal_contact_t contact;
        struct gcal_contact_array all_contacts;
        int result;
        size_t i;

        /* Create a gcal 'object' and authenticate with server */
        if (!(gcal = gcal_new(GCONTACT)))
                exit(1);
        if (argc == 3)
                result = gcal_get_authentication(gcal, argv[1], argv[2]);
        else
                result = gcal_get_authentication(gcal, "username",
                                                 "password");

        /* Get all contacts and print its name/prefered email/updated time */
        result = gcal_get_contacts(gcal, &all_contacts);

        for (i = 0; i < all_contacts.length; ++i) {
                contact = gcal_contact_element(&all_contacts, i);
                if (!contact)
                        break;

                printf("contact: %d\ttitle:%s\temail:%s\tupdated:%s\n",
                       i,
                       gcal_contact_get_title(contact),
                       gcal_contact_get_email(contact),
                       gcal_contact_get_updated(contact));

        }

        /* Cleanup */
        gcal_cleanup_contacts(&all_contacts);
        gcal_delete(gcal);

        return 0;
}


```


**Add/edit/delete a calendar event**
```
#include <stdio.h>
#include <gcalendar.h>

int main(int argc, char *argv[])
{
        gcal_t gcal;
        gcal_event_t event;
        int result;

        /* Create a gcal 'object' and authenticate with server */
        if (!(gcal = gcal_new(GCALENDAR)))
                exit(1);

        if (argc == 3)
                result = gcal_get_authentication(gcal, argv[1], argv[2]);
        else
                result = gcal_get_authentication(gcal, "username",
                                                 "password");

        /* Create an event 'object' and fill in some data */
        if ((event = gcal_event_new(NULL))) {
                gcal_event_set_title(event, "A new event");
                gcal_event_set_content(event, "Here goes the description");
                gcal_event_set_start(event, "2008-06-26T16:00:00Z");
                gcal_event_set_end(event, "2008-06-26T18:00:00Z");
                gcal_event_set_where(event, "A nice place for a meeting");
        }

        /* Add a new event */
        if (!(result = gcal_add_event(gcal, event))) {

                /* Edit this event */
                gcal_event_set_title(event, "Changing the title");
                result = gcal_update_event(gcal, event);

                /* Delete this event (note: google doesn't really deletes
                 * the event, but set its status to 'cancelled' and keeps
                 * then for nearly 4 weeks).
                 */
                result = gcal_erase_event(gcal, event);
        }

        /* Cleanup */
        gcal_event_delete(event);
        gcal_delete(gcal);

        return 0;
}

```


**Query for updated calendar events**
```
#include <stdio.h>
#include <gcalendar.h>

int main(int argc, char *argv[])
{
        gcal_t gcal;
        struct gcal_event_array event_array;
        int result;

        /* Create a gcal 'object' and authenticate with server */
        if (!(gcal = gcal_new(GCALENDAR)))
                exit(1);

        if (argc == 3)
                result = gcal_get_authentication(gcal, argv[1], argv[2]);
        else
                result = gcal_get_authentication(gcal, "username",
                                                 "password");

        /* This will query for all updated events (fall in this category
         * added/deleted/updated events) starting for 06:00Z UTC of today).
         */
        if (!(result = gcal_get_updated_events(gcal, &event_array, NULL)))
                printf("updated events: %d\n", (int)event_array.length);

        /* Cleanup */
        gcal_cleanup_events(&event_array);
        gcal_delete(gcal);

        return 0;

}

```

**Add a new contact from a raw XML file, checking for error codes**
```
#include <stdio.h>
#include <gcalendar.h>
#include <gcal_status.h>

int main(int argc, char *argv[])
{
        char *super_contact = NULL;
        const char filename[] = "supercontact.xml";
        gcal_t gcal;
        int result;

        gcal = gcal_new(GCONTACT);
        result = gcal_get_authentication(gcal, "username", "password");

        /* find_load_file is a function that will magically load the contents
         * of a XML file into a string.
         */
         if (find_load_file(filename, &super_contact))
                 exit(-1); */

        result = gcal_add_xmlentry(gcal, super_contact, NULL);
        if (result == -1)
                /* Check for errors and print status code */
                printf("Failed adding a new contact! HTTP code: %d"
                "\nmsg: %s\n", gcal_status_httpcode(gcal),
                gcal_status_msg(gcal));

        /* Cleanup */
        free(super_contact);
        gcal_delete(gcal);

        return 0;
}

```