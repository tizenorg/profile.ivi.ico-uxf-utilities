#ifndef _ICO_DBUS_AMB_EFL_H_
#define _ICO_DBUS_AMB_EFL_H_
#include <dbus/dbus.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int dbus_type;

union dbus_value_variant {
        unsigned char yval;
        dbus_bool_t bval;
        dbus_int16_t i16val;
        dbus_uint16_t ui16val;
        dbus_int32_t i32val;
        dbus_uint32_t ui32val;
        dbus_int64_t i64val;
        dbus_uint64_t ui64val;
        double dval;
        char *sval;
};

typedef struct _ico_dbus_error
{
  const char *name;    /**< public error name field */
  const char *message; /**< public error message field */
} ico_dbus_error_t;

typedef void (*ico_dbus_amb_getcb)(const char *objectname, const char *property, dbus_type type, union dbus_value_variant value, void *user_data);
typedef void (*ico_dbus_amb_noticb)(const char *objectname, const char *property, dbus_type type, union dbus_value_variant value, int sequence, struct timeval tv, void *user_data);

typedef void (*ico_dbus_amb_findcb)(const char *objectname,
                                    const char *property,
                                    dbus_type type,
                                    void *user_data,
                                    ico_dbus_error_t *error);

int ico_dbus_amb_start(void);
int ico_dbus_amb_end(void);
int ico_dbus_amb_get(const char *objectname, const char *property, int zone, dbus_type type, ico_dbus_amb_getcb cb, void *user_data);
//int ico_dbus_amb_set(const char *objectname, const char *property, int zone, dbus_type type, union dbus_value_variant value);
int ico_dbus_amb_subscribe(const char *objectname, const char *property, int zone, dbus_type type, ico_dbus_amb_noticb cb, void *user_data);
int ico_dbus_amb_unsubscribe(const char *objectname, const char *property, int zone);
//int ico_dbus_amb_gethistory(const char *objectname, const char *property, int zone);
//int ico_dbus_amb_getlist(void);
int ico_dbus_amb_find_property(const char *objectname,
                               const char *property,
                               int zone,
                               dbus_type type,
                               ico_dbus_amb_findcb cb,
                               void *user_data);
#ifdef __cplusplus
}
#endif
#endif //_ICO_DBUS_AMB_EFL_H_
