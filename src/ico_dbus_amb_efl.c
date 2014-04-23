#include <E_DBus.h>
#include <stdio.h>

#include "ico_log.h"
#include "ico_dbus_amb_efl.h"

#define LATER1024

/*============================================================================*/
/* global API                                                                 */
/*============================================================================*/
#if defined(__GNUC__) && __GNUC__ >= 4
#define ICO_API __attribute__ ((visibility("default")))
#else
#define ICO_API
#endif

E_DBus_Connection *conn = NULL;
static const char gBUSNAME[] = "org.automotive.message.broker";
static const char gAMBManagerPath[] = "org.automotive.Manager";
static const char gAMBManagerIf[] = "org.automotive.Manager";
static const char gAMBInterface[] = "org.automotive.";
static const char gAMBPROPERTIES_IF[] = "org.freedesktop.DBus.Properties";

enum MethodType {
    METHOD_SET,
    METHOD_GET,
    METHOD_GETHISTORY,
    METHOD_SUBSCRIBE,
    METHOD_UNSUBSCRIBE,
    METHOD_FIND,
};

struct _method_args {
    char *objectname;
    char *property;
    int zone;
    dbus_type dtype;
    union dbus_value_variant value;
    ico_dbus_amb_getcb getcb;
    ico_dbus_amb_noticb noticb;
    ico_dbus_amb_findcb findcb;
    enum MethodType mtype;
    void *user_data;
};

struct _signal_handler {
    char *objectname;
    char *property;
    int zone;
    E_DBus_Signal_Handler *handler;
    struct _method_args *args;
    struct _signal_handler *next;
};

static struct _signal_handler *signal_handler_list = NULL;

static int getproperty(struct _method_args *args);
static void getproperty_cb(void *data, DBusMessage *msg, DBusError *error);
static void set_cb(void *data, DBusMessage *msg, DBusError *error);
static void get_cb(void *data, DBusMessage *msg, DBusError *error);
static void subscribe_cb(void *data, DBusMessage *msg);

ICO_API int ico_dbus_amb_start(void) {
    int ret;
    ret =  e_dbus_init();
    if (ret == 0) {
        return -1;
    }
    conn = e_dbus_bus_get(DBUS_BUS_SYSTEM);
    if (conn == NULL) {
        return -1;
    }
    return 0;
}

ICO_API int ico_dbus_amb_end(void) {
    if (conn != NULL) {
        e_dbus_connection_close(conn);
    }
    return 0;
}

ICO_API int ico_dbus_amb_get(const char *objectname, const char *property, int zone, dbus_type type, ico_dbus_amb_getcb cb, void *user_data) {
    struct _method_args *args;

    if (objectname == NULL || strlen(objectname) == 0) {
        return -1;
    }
    if (property == NULL || strlen(property) == 0) {
        return -1;
    }
    args = (struct _method_args*)malloc(sizeof(struct _method_args));
    if (args == NULL) {
        return -1;
    }
    args->objectname = strdup(objectname);
    args->property = strdup(property);
    args->getcb = cb;
    args->zone = zone;
    args->dtype = type;
    args->mtype = METHOD_GET;
    args->user_data = user_data;

    return getproperty(args);
}

#if 0
ICO_API int ico_dbus_amb_set(const char *objectname, const char *property, int zone, dbus_type type, union dbus_value_variant value) {
    struct _method_args *args;

    if (objectname == NULL || strlen(objectname) == 0) {
        return -1;
    }
    if (property == NULL || strlen(property) == 0) {
        return -1;
    }
    args = (struct _method_args*)malloc(sizeof(struct _method_args));
    if (args == NULL) {
        return -1;
    }
    args->objectname = strdup(objectname);
    args->property = strdup(property);
    args->zone = zone;
    args->dtype = type;
    args->mtype = METHOD_SET;
    args->value = value;

    return getproperty(args);
}
#endif

ICO_API int ico_dbus_amb_subscribe(const char *objectname, const char *property, int zone, dbus_type type, ico_dbus_amb_noticb cb, void *user_data) {
    struct _method_args *args;
    struct _signal_handler *head;

    if (objectname == NULL || strlen(objectname) == 0) {
        return -1;
    }
    if (property == NULL || strlen(property) == 0) {
        return -1;
    }

    if (signal_handler_list != NULL) {
        head = signal_handler_list;
        while (head != NULL) {
            if (strcmp(head->property, property) == 0 && head->zone == zone && strcmp(head->objectname, objectname) == 0) {
                return -2;
            }
            head = head->next;
        }
    }

    args = (struct _method_args*)malloc(sizeof(struct _method_args));
    if (args == NULL) {
        return -1;
    }
    args->objectname = strdup(objectname);
    args->property = strdup(property);
    args->noticb = cb;
    args->zone = zone;
    args->dtype = type;
    args->mtype = METHOD_SUBSCRIBE;
    args->user_data = user_data;

    return getproperty(args);
}

ICO_API int ico_dbus_amb_unsubscribe(const char *objectname, const char *property, int zone) {
    struct _signal_handler *head, *prev;

    if (objectname == NULL || strlen(objectname) == 0) {
        return -1;
    }
    if (property == NULL || strlen(property) == 0) {
        return -1;
    }

    if (signal_handler_list != NULL) {
        prev = NULL;
        head = signal_handler_list;
        while (head != NULL) {
            if (strcmp(head->property, property) == 0 && head->zone == zone && strcmp(head->objectname, objectname) == 0) {
                if (prev == NULL) {
                    signal_handler_list = head->next;
                }
                else {
                    prev->next = head->next;
                }
                e_dbus_signal_handler_del(conn, head->handler);
                free(head->objectname);
                free(head->property);
                free(head->args->objectname);
                free(head->args->property);
                free(head->args);
                free(head);
                return 0;
            }
            prev = head;
            head = head->next;
        }
    }
    return -2;
}

#if 0
ICO_API int ico_dbus_amb_gethistory(const char *objectname, const char *property, int zone) {
    return 0;
}
#endif

ICO_API int ico_dbus_amb_find_property(const char *objectname,
                                       const char *property,
                                       int zone,
                                       dbus_type type,
                                       ico_dbus_amb_findcb cb,
                                       void *user_data)
{
    struct _method_args *args;

    if (objectname == NULL || strlen(objectname) == 0) {
        return -1;
    }
    if (property == NULL || strlen(property) == 0) {
        return -1;
    }
    args = (struct _method_args*)malloc(sizeof(struct _method_args));
    if (args == NULL) {
        return -1;
    }
    args->objectname = strdup(objectname);
    args->property = strdup(property);
    args->findcb = cb;
    args->zone = zone;
    args->dtype = type;
    args->mtype = METHOD_FIND;
    args->user_data = user_data;

    return getproperty(args);
}

int getproperty(struct _method_args *args) {
    DBusMessage *msg;
#ifdef LATER1024
    msg = dbus_message_new_method_call(gBUSNAME, "/", gAMBManagerIf, "FindObjectForZone");
    if (msg == NULL) {
        return -1;
    }
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &(args->objectname), DBUS_TYPE_INT32, &(args->zone), DBUS_TYPE_INVALID);
#else
    msg = dbus_message_new_method_call(gBUSNAME, "/", gAMBManagerIf, "findProperty");
    if (msg == NULL) {
        return -1;
    }
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &(args->objectname), DBUS_TYPE_INVALID);
#endif
    e_dbus_message_send(conn, msg, getproperty_cb, -1, (void*)args);
    dbus_message_unref(msg);
    return 0;
}

void getproperty_cb(void *data, DBusMessage *msg, DBusError *error) {
    struct _method_args *args;
    DBusError e;
    char *path;
    DBusMessage *methodmsg;
    char interface[128];
    char *interfacep;
    E_DBus_Signal_Handler *sighandler;
    struct _signal_handler *_sig_handler, *head;
    struct timeval tv;

    args = (struct _method_args*)data;

    if (!msg) {
        if (NULL != args && NULL != error) {
            ICO_ERR("FindProperty[%s]: %s : %s", args->property,
                    error->name, error->message);
            if (METHOD_FIND == args->mtype) {
                ico_dbus_error_t ico_error;
                ico_error.name = error->name;
                ico_error.message = error->message;
                if (NULL != args->findcb) {
                    args->findcb(args->objectname, args->property, args->dtype,
                                 args->user_data, &ico_error);
                }
                free(args->objectname);
                free(args->property);
                free(args);
            }
        }
        else {
            ICO_ERR("FindProperty: %s : %s", error->name, error->message);
        }
        return;
    }
    dbus_error_init(&e);

    dbus_message_get_args(msg, &e, DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID);
    //printf("Object Path:%s\n", path);
    memset(interface, 0, sizeof(interface));
    sprintf(interface, "%s%s", gAMBInterface, args->objectname);
    interfacep = interface;

    gettimeofday(&tv, NULL);
    printf("%d.%06d LIB FINDPROPERTY %s %s\n", (int)tv.tv_sec, (int)tv.tv_usec, args->property, path);

    switch(args->mtype) {
    case METHOD_SET :
        methodmsg = dbus_message_new_method_call(gBUSNAME, path, gAMBPROPERTIES_IF, "Set");
        switch (args->dtype) {
        case DBUS_TYPE_BYTE :
            dbus_message_append_args(methodmsg, DBUS_TYPE_STRING, &interfacep, DBUS_TYPE_STRING, &(args->property), DBUS_TYPE_BYTE, &(args->value.yval), DBUS_TYPE_INVALID);
            break;
        case DBUS_TYPE_BOOLEAN :
            dbus_message_append_args(methodmsg, DBUS_TYPE_STRING, &interfacep, DBUS_TYPE_STRING, &(args->property), DBUS_TYPE_BOOLEAN, &(args->value.bval), DBUS_TYPE_INVALID);
            break;
        case DBUS_TYPE_INT16 :
            dbus_message_append_args(methodmsg, DBUS_TYPE_STRING, &interfacep, DBUS_TYPE_STRING, &(args->property), DBUS_TYPE_INT16, &(args->value.i16val), DBUS_TYPE_INVALID);
            break;
        case DBUS_TYPE_UINT16 :
            dbus_message_append_args(methodmsg, DBUS_TYPE_STRING, &interfacep, DBUS_TYPE_STRING, &(args->property), DBUS_TYPE_UINT16, &(args->value.ui16val), DBUS_TYPE_INVALID);
            break;
        case DBUS_TYPE_INT32 :
            dbus_message_append_args(methodmsg, DBUS_TYPE_STRING, &interfacep, DBUS_TYPE_STRING, &(args->property), DBUS_TYPE_INT32, &(args->value.i32val), DBUS_TYPE_INVALID);
            break;
        case DBUS_TYPE_UINT32 :
            dbus_message_append_args(methodmsg, DBUS_TYPE_STRING, &interfacep, DBUS_TYPE_STRING, &(args->property), DBUS_TYPE_UINT32, &(args->value.ui32val), DBUS_TYPE_INVALID);
            break;
        case DBUS_TYPE_INT64 :
            dbus_message_append_args(methodmsg, DBUS_TYPE_STRING, &interfacep, DBUS_TYPE_STRING, &(args->property), DBUS_TYPE_INT64, &(args->value.i64val), DBUS_TYPE_INVALID);
            break;
        case DBUS_TYPE_UINT64 :
            dbus_message_append_args(methodmsg, DBUS_TYPE_STRING, &interfacep, DBUS_TYPE_STRING, &(args->property), DBUS_TYPE_UINT64, &(args->value.ui64val), DBUS_TYPE_INVALID);
            break;
        case DBUS_TYPE_DOUBLE :
            dbus_message_append_args(methodmsg, DBUS_TYPE_STRING, &interfacep, DBUS_TYPE_STRING, &(args->property), DBUS_TYPE_DOUBLE, &(args->value.dval), DBUS_TYPE_INVALID);
            break;
        case DBUS_TYPE_STRING :
            dbus_message_append_args(methodmsg, DBUS_TYPE_STRING, &interfacep, DBUS_TYPE_STRING, &(args->property), DBUS_TYPE_STRING, &(args->value.sval), DBUS_TYPE_INVALID);
            break;
        default :
            break;
        }
        e_dbus_message_send(conn, methodmsg, set_cb, -1, NULL);
        dbus_message_unref(methodmsg);
        free(args->objectname);
        free(args->property);
        free(args);
        break;
    case METHOD_GET :
        methodmsg = dbus_message_new_method_call(gBUSNAME, path, gAMBPROPERTIES_IF, "Get");
        dbus_message_append_args(methodmsg, DBUS_TYPE_STRING, &interfacep, DBUS_TYPE_STRING, &(args->property), DBUS_TYPE_INVALID);
        e_dbus_message_send(conn, methodmsg, get_cb, -1, (void*)args);
        dbus_message_unref(methodmsg);
        break;
    case METHOD_GETHISTORY :
        break;
    case METHOD_SUBSCRIBE :
        _sig_handler = (struct _signal_handler*)malloc(sizeof(struct _signal_handler));
        if (_sig_handler == NULL) {
            free(args->objectname);
            free(args->property);
            free(args);
            return;
        }
        sighandler = e_dbus_signal_handler_add(conn, gBUSNAME, path, gAMBPROPERTIES_IF, "PropertiesChanged", subscribe_cb, (void*)args);
        _sig_handler->property = strdup(args->property);
        _sig_handler->objectname = strdup(args->objectname);
        _sig_handler->zone = args->zone;
        _sig_handler->handler = sighandler;
        _sig_handler->args = args;
        _sig_handler->next = NULL;
        if (signal_handler_list == NULL) {
            signal_handler_list = _sig_handler;
        }
        else {
            head = signal_handler_list;
            while (head->next != NULL) {
                head = head->next;
            }
            head->next = _sig_handler;
        }
        break;
    case METHOD_UNSUBSCRIBE :
        break;
    case METHOD_FIND :
        if (NULL != args->findcb) {
            args->findcb(args->objectname, args->property, args->dtype,
                         args->user_data, NULL);
            free(args->objectname);
            free(args->property);
            free(args);
        }
        break;
    default:
        break;
    }
}

void set_cb(void *data, DBusMessage *msg, DBusError *error) {
    if (!msg) {
        if (error) {
            fprintf(stderr, "set_cb::DBusError [%s: %s]\n", error->name, error->message);
        }
        return;
    }
}

void get_cb(void *data, DBusMessage *msg, DBusError *error) {
    struct _method_args *args;
    DBusError e;
    union dbus_value_variant value;
    DBusMessageIter iter, variant;

    if (!msg) {
        if (error) {
            fprintf(stderr, "get_cb::DBusError [%s: %s]\n", error->name, error->message);
        }
        return;
    }
    if (!msg || !dbus_message_iter_init(msg, &iter)) {
        return;
    }
    dbus_error_init(&e);
    args = (struct _method_args*)data;
    dbus_message_iter_recurse(&iter, &variant);

    switch (args->dtype) {
    case DBUS_TYPE_BYTE :
        dbus_message_iter_get_basic(&variant, &value.yval);
        break;
    case DBUS_TYPE_BOOLEAN :
        dbus_message_iter_get_basic(&variant, &value.bval);
        break;
    case DBUS_TYPE_INT16 :
        dbus_message_iter_get_basic(&variant, &value.i16val);
        break;
    case DBUS_TYPE_UINT16 :
        dbus_message_iter_get_basic(&variant, &value.ui16val);
        break;
    case DBUS_TYPE_INT32 :
        dbus_message_iter_get_basic(&variant, &value.i32val);
        break;
    case DBUS_TYPE_UINT32 :
        dbus_message_iter_get_basic(&variant, &value.ui32val);
        break;
    case DBUS_TYPE_INT64 :
        dbus_message_iter_get_basic(&variant, &value.i64val);
        break;
    case DBUS_TYPE_UINT64 :
        dbus_message_iter_get_basic(&variant, &value.ui64val);
        break;
    case DBUS_TYPE_DOUBLE :
        dbus_message_iter_get_basic(&variant, &value.dval);
        break;
    case DBUS_TYPE_STRING :
        dbus_message_iter_get_basic(&variant, &value.sval);
        break;
    default :
        break;
    }
    args->getcb(args->objectname, args->property, args->dtype, value, args->user_data);
    free(args->objectname);
    free(args->property);
    free(args);
}

static void subscribe_cb(void *data, DBusMessage *msg) {
    struct _method_args *args;
    DBusError e;
    union dbus_value_variant value;
    char *propertyname;
    char *sequencename;
    char *timename;
    int sequenceno;
    double time;
    struct timeval tv;
    int time_s;
    int time_ms;
    DBusMessageIter iter, dict, entry, variant;

    if (!msg || !dbus_message_iter_init(msg, &iter)) {
        return;
    }
    dbus_error_init(&e);
    args = (struct _method_args*)data;

    dbus_message_iter_next(&iter);
    dbus_message_iter_recurse(&iter, &dict);
    dbus_message_iter_recurse(&dict, &entry);
    dbus_message_iter_get_basic(&entry, &propertyname);
    if (strcmp(args->property, propertyname) != 0) {
        return;
    }
    dbus_message_iter_next(&entry);
    dbus_message_iter_recurse(&entry, &variant);

    switch (args->dtype) {
    case DBUS_TYPE_BYTE :
        dbus_message_iter_get_basic(&variant, &value.yval);
        break;
    case DBUS_TYPE_BOOLEAN :
        dbus_message_iter_get_basic(&variant, &value.bval);
        break;
    case DBUS_TYPE_INT16 :
        dbus_message_iter_get_basic(&variant, &value.i16val);
        break;
    case DBUS_TYPE_UINT16 :
        dbus_message_iter_get_basic(&variant, &value.ui16val);
        break;
    case DBUS_TYPE_INT32 :
        dbus_message_iter_get_basic(&variant, &value.i32val);
        break;
    case DBUS_TYPE_UINT32 :
        dbus_message_iter_get_basic(&variant, &value.ui32val);
        break;
    case DBUS_TYPE_INT64 :
        dbus_message_iter_get_basic(&variant, &value.i64val);
        break;
    case DBUS_TYPE_UINT64 :
        dbus_message_iter_get_basic(&variant, &value.ui64val);
        break;
    case DBUS_TYPE_DOUBLE :
        dbus_message_iter_get_basic(&variant, &value.dval);
        break;
    case DBUS_TYPE_STRING :
        dbus_message_iter_get_basic(&variant, &value.sval);
        break;
    default :
        break;
    }
    dbus_message_iter_next(&dict);
    dbus_message_iter_recurse(&dict, &entry);
    dbus_message_iter_get_basic(&entry, &sequencename);
    dbus_message_iter_next(&entry);
    dbus_message_iter_recurse(&entry, &variant);
    dbus_message_iter_get_basic(&variant, &sequenceno);
    dbus_message_iter_next(&dict);
    dbus_message_iter_recurse(&dict, &entry);
    dbus_message_iter_get_basic(&entry, &timename);
    dbus_message_iter_next(&entry);
    dbus_message_iter_recurse(&entry, &variant);
    dbus_message_iter_get_basic(&variant, &time);
    //printf("Subscribe %s, %s:%d, %s:%f\n", propertyname, sequencename, sequenceno, timename, time);
    time_s = (int)time;
    time -= time_s;
    time_ms = time * 1000000;
    tv.tv_sec = time_s;
    tv.tv_usec = time_ms;
    args->noticb(args->objectname, args->property, args->dtype, value, sequenceno, tv, args->user_data);
}
