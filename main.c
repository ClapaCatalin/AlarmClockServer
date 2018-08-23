#include <stdio.h>
#include <glib.h>
#include <time.h>
#include "alarmClockStubs.h"

alarmClockAlarmClock* interfacestructthing;
static const char* name="com.time.service.alarmClock";


gchar* LocalAlarmTime;
gboolean AlarmStatus = FALSE;

typedef struct TimeStruct
{
    gchar* TimeString;
    time_t TimeWhenSet;
    int TotalMinutes;
}TimeStruct;
TimeStruct LocalTimeStruct;
TimeStruct TS;

//Helper Functions -----------------------------------------
time_t gettime()
{
    time_t seconds;
    seconds=time(NULL);
    return seconds;
}

int minutesfromstring(gchar* time)
{
    int hhour= time[0] - '0';
    int lhour= time[1] - '0';
    int hour= (10*hhour) + lhour;

    int hminute = time[2] - '0';
    int lminute = time[3] - '0';
    int minute= 10*hminute + lminute;

    int result= hour*60 + minute;

    return result;
}
//----------------------------------------------------------

gboolean CheckTimings()
{
    //printf("Checking Timing ...\n");
    int Check = gettime() / 60; // from fix point until now in minutes
    if(AlarmStatus)
    {
        if(Check == TS.TimeWhenSet / 60) //  present vs alarmtime in minutes
        {
            alarm_clock_alarm_clock_emit_ring_alarm(interfacestructthing,"RINGRING\n");
            printf("RINGRING\n");
            AlarmStatus=FALSE;
        }
    }
    return TRUE;
}

gboolean localgetalarmstatus(alarmClockAlarmClock* interfacestructthing, GDBusMethodInvocation *inv, gboolean* out)
{
    gboolean r=TRUE;
    if(AlarmStatus)
    {
        printf("ON\n");
        r=TRUE;
    }
    else
    {
        printf("OFF\n");
        r=FALSE;
    }
    alarm_clock_alarm_clock_complete_get_alarm_status(interfacestructthing,inv,r);
    return TRUE;

}

gboolean localsetTime(alarmClockAlarmClock* interfacestructthing, GDBusMethodInvocation *inv, gchar* time)
{
    LocalTimeStruct.TimeString = time;
    LocalTimeStruct.TimeWhenSet=gettime();
    LocalTimeStruct.TotalMinutes=minutesfromstring(time);
    printf("Time Set to %s \n", LocalTimeStruct.TimeString);
    printf("Seconds from 1970: %ld \n",LocalTimeStruct.TimeWhenSet);
    printf("Current time %s in minutes %d \n",LocalTimeStruct.TimeString,LocalTimeStruct.TotalMinutes);
    alarm_clock_alarm_clock_complete_set_time(interfacestructthing,inv);
    return TRUE;
}

gboolean localsetAlarmTime(alarmClockAlarmClock* interfacestructthing, GDBusMethodInvocation *inv, gchar* alarm)
{
    int Timedifference = minutesfromstring(alarm) - LocalTimeStruct.TotalMinutes; //minute difference between alarm and curent time

    printf("Time difference:%d \n",Timedifference);
    if(Timedifference < 0)
    {
        //24*60 = 1440 min in a day
        TS.TimeString= alarm;
        TS.TimeWhenSet=LocalTimeStruct.TimeWhenSet + 1440*60 + Timedifference*60;
        TS.TotalMinutes= minutesfromstring(alarm);
        printf("Alarm time set for tomorrow at: %s !\n",TS.TimeString);
    }
    else
    {
        TS.TimeString= alarm; //alarm time in string format EX: 1305
        TS.TimeWhenSet=LocalTimeStruct.TimeWhenSet + Timedifference*60; // time from fix point to future alarm point (relative to current time) in SECONDS
        TS.TotalMinutes= minutesfromstring(alarm);
        printf("Alarm time set to:%s \n", TS.TimeString);
        printf("Alarm - Seconds from 1970: %ld \n",TS.TimeWhenSet);
        printf("Alarm time %s in minutes %d \n",TS.TimeString,TS.TotalMinutes);
    }
    alarm_clock_alarm_clock_complete_set_alarm_time(interfacestructthing,inv);
    return TRUE;
}

gboolean localsetAlarmStatus(alarmClockAlarmClock* interfacestructthing, GDBusMethodInvocation *inv, gboolean status)
{
    AlarmStatus=status;
    printf("Alarm status is :%d \n",AlarmStatus);
    alarm_clock_alarm_clock_complete_set_alarm_status(interfacestructthing,inv);
     return TRUE;
}

void LinkMethodsToLocal(alarmClockAlarmClock* interfacestructthing)
{
    g_signal_connect(interfacestructthing,"handle-get-alarm-status",G_CALLBACK(localgetalarmstatus),NULL);
    g_signal_connect(interfacestructthing,"handle-set-time",G_CALLBACK(localsetTime),NULL);
    g_signal_connect(interfacestructthing,"handle-set-alarm-time",G_CALLBACK(localsetAlarmTime),NULL);
    g_signal_connect(interfacestructthing,"handle-set-alarm-status",G_CALLBACK(localsetAlarmStatus),NULL);
}


static void BusAquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{

    alarmClockObjectSkeleton *object;
    GDBusObjectManagerServer *manager = NULL;

    manager = g_dbus_object_manager_server_new("/com/time/service/manager");
    object = alarm_clock_object_skeleton_new("/com/time/service/manager/Object");
    interfacestructthing = alarm_clock_alarm_clock_skeleton_new();

    alarm_clock_object_skeleton_set_alarm_clock(object, interfacestructthing);
    g_object_unref(interfacestructthing);

    //Callback attempt #1 ever GLHF

    LinkMethodsToLocal(interfacestructthing);

    //back to Aquisition

    g_dbus_object_manager_server_export(manager, G_DBUS_OBJECT_SKELETON(object));

    g_object_unref(object);

    g_dbus_object_manager_server_set_connection(manager,connection);

    //printf("succesfully aquired name:%s\n",name);


}

static void NameAquired(GDBusConnection *connection,const char* name,gpointer user_data)
{
    printf("NameAquired %s \n",name);
}

static void NameLost(GDBusConnection *connection,const char* name,gpointer user_data)
{
    printf("NameLost %s \n",name);

}


void init()
{
    printf("Starting init()\n");
    guint id= g_bus_own_name(
                G_BUS_TYPE_SYSTEM,
                name,
                G_BUS_NAME_OWNER_FLAGS_NONE,
                BusAquired,
                NameAquired,
                NameLost,
                NULL,
                NULL);
    printf("init success yey! id:%d \n",id);
}

int main()
{
    GMainLoop *MainLoop = g_main_loop_new(NULL,FALSE);
    if(MainLoop == NULL)
    {
        printf("GMainLoop failed\n");
    }

    init();

    gint func_ref= g_timeout_add(1000,CheckTimings,NULL);

    printf("starting gmain loop\n");
    g_main_loop_run(MainLoop);

    g_source_remove(func_ref);
    return 0;
}
