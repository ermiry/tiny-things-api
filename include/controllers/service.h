#ifndef _THINGS_SERVICE_H_
#define _THINGS_SERVICE_H_

struct _HttpResponse;

extern struct _HttpResponse *missing_values;

extern struct _HttpResponse *things_works;
extern struct _HttpResponse *current_version;

extern struct _HttpResponse *catch_all;

extern unsigned int things_service_init (void);

extern void things_service_end (void);

#endif