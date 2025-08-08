#include "panelproxy.h"

#include <ctype.h>
#include <libintl.h>

#include <dbus/dbus.h>

#include <fcitx/candidate.h>
#include <fcitx/hook.h>
#include <fcitx/module/dbus/fcitx-dbus.h>

#include "freewubi-config.h"
#include "inputstate.h"
#include "ipc/ipc.h"

static inline boolean CheckAddPrefix( const char** name) {
    boolean result = !((*name)[0] == '\0' || (*name)[0] == '/' || (*name)[0] == '@');
    if ((*name)[0] == '@') {
        (*name) += 1;
    }

    return result;
}

static const char* freewubipanel_introspection_xml =
    "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" "
    "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">"
    "<node name=\"" FREEWUBI_INPUTMETHOD_OBJECTPATH "\">"
    "<interface name=\"org.freedesktop.DBus.Introspectable\">"
    "<method name=\"Introspect\">"
    "<arg name=\"data\" direction=\"out\" type=\"s\"/>"
    "</method>"
    "</interface>"
    "<interface name=\"" FREEWUBI_INPUTMETHOD_SERVICENAME "\">"
    "<signal name=\"ExecDialog\">"
    "<arg name=\"prop\" direction=\"in\" type=\"s\"/>"
    "</signal>"
    "<signal name=\"ExecMenu\">"
    "<arg name=\"prop\" direction=\"in\" type=\"as\"/>"
    "</signal>"
    "<signal name=\"RegisterProperties\">"
    "<arg name=\"prop\" direction=\"in\" type=\"as\"/>"
    "</signal>"
    "<signal name=\"UpdateProperty\">"
    "<arg name=\"prop\" direction=\"in\" type=\"s\"/>"
    "</signal>"
    "<signal name=\"RemoveProperty\">"
    "<arg name=\"prop\" direction=\"in\" type=\"s\"/>"
    "</signal>"
    "<signal name=\"ShowAux\">"
    "<arg name=\"toshow\" direction=\"in\" type=\"b\"/>"
    "</signal>"
    "<signal name=\"ShowPreedit\">"
    "<arg name=\"toshow\" direction=\"in\" type=\"b\"/>"
    "</signal>"
    "<signal name=\"ShowLookupTable\">"
    "<arg name=\"toshow\" direction=\"in\" type=\"b\"/>"
    "</signal>"
    "<signal name=\"UpdateLookupTable\">"
    "<arg name=\"label\" direction=\"in\" type=\"as\"/>"
    "<arg name=\"text\" direction=\"in\" type=\"as\"/>"
    "</signal>"
    "<signal name=\"UpdateLookupTableCursor\">"
    "<arg name=\"cursor\" direction=\"in\" type=\"i\"/>"
    "</signal>"
    "<signal name=\"UpdatePreeditCaret\">"
    "<arg name=\"position\" direction=\"in\" type=\"i\"/>"
    "</signal>"
    "<signal name=\"UpdatePreeditText\">"
    "<arg name=\"text\" direction=\"in\" type=\"s\"/>"
    "<arg name=\"attr\" direction=\"in\" type=\"s\"/>"
    "</signal>"
    "<signal name=\"UpdateAux\">"
    "<arg name=\"text\" direction=\"in\" type=\"s\"/>"
    "<arg name=\"attr\" direction=\"in\" type=\"s\"/>"
    "</signal>"
    "<signal name=\"UpdateSpotLocation\">"
    "<arg name=\"x\" direction=\"in\" type=\"i\"/>"
    "<arg name=\"y\" direction=\"in\" type=\"i\"/>"
    "</signal>"
    "<signal name=\"UpdateScreen\">"
    "<arg name=\"screen\" direction=\"in\" type=\"i\"/>"
    "</signal>"
    "<signal name=\"Enable\">"
    "<arg name=\"toenable\" direction=\"in\" type=\"b\"/>"
    "</signal>"
    "</interface>"
    "</node>";

#define FREEWUBI_PANEL_BUFFER_SIZE 4096
#ifndef DBUS_TIMEOUT_USE_DEFAULT
#  define DBUS_TIMEOUT_USE_DEFAULT (-1)
#endif

#define INDICATOR_KEYBOARD_PREFIX "@indicator-keyboard-"
#define INDICATOR_KEYBOARD_LENGTH (strlen(INDICATOR_KEYBOARD_PREFIX))


static void FreeWubiPanelProxyMoveInputWindow();
static void FreeWubiPanelProxyShowAux(boolean toShow);
static void FreeWubiPanelProxyShowPreedit(boolean toShow);
static void FreeWubiPanelProxyShowLookupTable(boolean toShow);
static void FreeWubiPanelProxyUpdatePreeditText(char *text);
static void FreeWubiPanelProxyUpdateAux(char *text);
static void FreeWubiPanelProxyUpdatePreeditCaret(int position);
static void FreeWubiPanelProxyEnable(boolean toEnable);
static void FreeWubiPanelProxyRegisterProperties(char *props[], int n);
static void FreeWubiPanelProxyUpdateProperty(char *prop);
static void FreeWubiPanelProxyRegisterAllStatus();
static void FreeWubiPanelProxySetIMStatus();
static DBusHandlerResult FreeWubiPanelProxyDBusEventHandler(DBusConnection *connection, DBusMessage *message, void *user_data);
static DBusHandlerResult FreeWubiPanelProxyDBusFilter(DBusConnection *connection, DBusMessage *message, void *user_data);
static int FreeWubiPanelProxyCalCursorPos();
static char* FreeWubiPanelProxyStatus2String(FcitxInstance* instance, FcitxUIStatus* status);
static char* FreeWubiPanelProxyComplexStatus2String(FcitxInstance* instance, FcitxUIComplexStatus* status);
static void FreeWubiPanelProxyOwnerChanged(void* user_data, void* arg, const char* serviceName, const char* oldName, const char* newName);

static inline boolean isUnity()
{
    return fcitx_utils_strcmp0(getenv("XDG_CURRENT_DESKTOP"), "Unity") == 0;
}

static void SetIMIcon(FcitxInstance* instance, char** prop)
{
    char layout[] = INDICATOR_KEYBOARD_PREFIX "Xx";
    const char* icon;
    char* imname;
    char* description;
    char temp[LANGCODE_LENGTH + 1] = { '\0', };
    FCITX_UNUSED(temp);
    FcitxInputContext* ic = FcitxInstanceGetCurrentIC(instance);
    if (ic == NULL) {
        icon = "kbd";
        imname = _("No input window");
        description = _("No input window");
    } else if (FcitxInstanceGetCurrentStatev2(instance) == IS_ACTIVE) {
        FcitxIM* im = FcitxInstanceGetCurrentIM(instance);
        if (im) {
            if (strncmp(im->uniqueName, "fcitx-keyboard-",
                        strlen("fcitx-keyboard-")) == 0) {
                if (isUnity()) {
                    layout[INDICATOR_KEYBOARD_LENGTH] =
                        toupper(im->langCode[0]);
                    layout[INDICATOR_KEYBOARD_LENGTH + 1] =
                        tolower(im->langCode[1]);
                    icon = layout;
                } else {
                    icon = "";
                }
                imname = im->uniqueName + strlen("fcitx-keyboard-");
            }
            else
            {
                icon = im->strIconName;
                imname = im->strName;
            }
            description = im->strName;
        } else {
            icon = "kbd";
            imname = _("Disabled");
            description = _("Input Method Disabled");
        }
    } else {
        icon = "kbd";
        imname = _("Disabled");
        description = _("Input Method Disabled");
    }
    /* add fcitx- prefix, unless icon name is an absolute path */

    char *icon_prefix;
    boolean result = CheckAddPrefix(&icon);
    if (result) {
        icon_prefix = ":fcitx-";
    } else {
        icon_prefix = ":";
    }
    fcitx_utils_alloc_cat_str(*prop, "/Fcitx/im:", imname, icon_prefix, icon,
                              ":", description, ":menu");
}

typedef struct {
    FcitxInstance* owner;
    DBusConnection* conn;
    int iOffsetY;
    int iOffsetX;
    FcitxMessages* messageUp;
    FcitxMessages* messageDown;
    int iCursorPos;
    int lastUpdateY;
    int lastUpdateX;
    int lastUpdateW;
    int lastUpdateH;
    int lastCursor;
} FreeWubiPanelProxy;

static void FreeWubiPanelProxyServiceExistCallback(DBusPendingCall *call, void *data);

static FreeWubiPanelProxy* freeWubiPanel = NULL;
void FreeWubiPanelProxyInitializeInstance(FcitxInstance* instance) {
    if (freeWubiPanel != NULL) {
        FreeWubiPanelProxyDestroyInstance();
    }

    freeWubiPanel = fcitx_utils_malloc0(sizeof(FreeWubiPanelProxy));

    freeWubiPanel->lastCursor = -2;
    freeWubiPanel->iCursorPos = 0;
    freeWubiPanel->owner = instance;
    freeWubiPanel->conn = FcitxDBusGetConnection(instance);

    DBusError err;
    dbus_error_init(&err);
    do {
        if (freeWubiPanel->conn == NULL) {
            // FcitxLog(ERROR, "DBus Not initialized");
            break;
        }

        dbus_bus_add_match(freeWubiPanel->conn,
                           "type='signal',sender='"FREEWUBI_PANEL_SERVICENAME"',interface='"FREEWUBI_PANEL_INTERFACE"'",
                           &err);
        dbus_connection_flush(freeWubiPanel->conn);
        if (dbus_error_is_set(&err)) {
            // FcitxLog(ERROR, "Match Error (%s)", err.message);
            break;
        }

        int id = FcitxDBusWatchName(instance, FREEWUBI_PANEL_SERVICENAME, freeWubiPanel,
                                    FreeWubiPanelProxyOwnerChanged, NULL, NULL);
        if (id == 0) {
            break;
        }

        if (!dbus_connection_add_filter(freeWubiPanel->conn, FreeWubiPanelProxyDBusFilter, freeWubiPanel, NULL)) {
            // FcitxLog(ERROR, "No memory");
            break;
        }

        DBusObjectPathVTable vtable = {NULL, &FreeWubiPanelProxyDBusEventHandler, NULL, NULL, NULL, NULL };

        dbus_connection_register_object_path(freeWubiPanel->conn, FREEWUBI_INPUTMETHOD_OBJECTPATH, &vtable, freeWubiPanel);

        freeWubiPanel->messageUp = FcitxMessagesNew();
        freeWubiPanel->messageDown = FcitxMessagesNew();

        const char* freeWubiPanelServiceName = FREEWUBI_PANEL_SERVICENAME;
        DBusMessage* message = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, "NameHasOwner");
        dbus_message_append_args(message, DBUS_TYPE_STRING, &freeWubiPanelServiceName, DBUS_TYPE_INVALID);

        DBusPendingCall *call = NULL;
        dbus_bool_t reply =
            dbus_connection_send_with_reply(freeWubiPanel->conn, message,
                                            &call, DBUS_TIMEOUT_USE_DEFAULT);
        if (reply == TRUE) {
            dbus_pending_call_set_notify(call,
                                         FreeWubiPanelProxyServiceExistCallback,
                                         freeWubiPanel,
                                         NULL);
            dbus_pending_call_unref(call);
        }
        dbus_connection_flush(freeWubiPanel->conn);
        dbus_message_unref(message);

        FreeWubiPanelProxyRegisterAllStatus();
        dbus_error_free(&err);
        return;
    } while (0);

    dbus_error_free(&err);
    free(freeWubiPanel);
}


void FreeWubiPanelProxyOwnerChanged(void* user_data, void* arg, const char* serviceName, const char* oldName, const char* newName) {
    /* old die and no new one */
    if (strcmp(serviceName, FREEWUBI_PANEL_SERVICENAME) == 0) {
        if (strlen(oldName) > 0 && strlen(newName) == 0) {
            FcitxLog(ERROR, "The FreeWubi UI service is unavailable!");
        }
    }
}

void FreeWubiPanelProxyServiceExistCallback(DBusPendingCall *call, void *data)
{
    FreeWubiPanelProxy *freeWubiPanel = (FreeWubiPanelProxy*)data;

    DBusMessage *msg = dbus_pending_call_steal_reply(call);

    if (msg) {
        dbus_bool_t has = FALSE;
        DBusError error;
        dbus_error_init(&error);
        dbus_message_get_args(msg, &error,
                              DBUS_TYPE_BOOLEAN, &has, DBUS_TYPE_INVALID);
        dbus_message_unref(msg);
        dbus_error_free(&error);
        if (!has) {
            FcitxLog(ERROR, "The FreeWubi UI service is unavailable!");
        }
    }
}

void FreeWubiPanelProxyRegisterAllStatus()
{
    FcitxInstance* instance = freeWubiPanel->owner;
    UT_array* uistats = FcitxInstanceGetUIStats(instance);
    UT_array* uicompstats = FcitxInstanceGetUIComplexStats(instance);
    char **prop = fcitx_utils_malloc0(sizeof(char*) * (2 + utarray_len(uistats) + utarray_len(uicompstats)));

    char *fcitx = _("Fcitx");
    fcitx_utils_alloc_cat_str(prop[0], "/Fcitx/logo:", fcitx, ":fcitx:", fcitx, ":menu");
    SetIMIcon(instance, &prop[1]);

    int count = 2;

    FcitxUIComplexStatus *compstatus;
    for (compstatus = (FcitxUIComplexStatus *) utarray_front(uicompstats);
         compstatus != NULL;
         compstatus = (FcitxUIComplexStatus *) utarray_next(uicompstats, compstatus)) {
        if (!compstatus->visible)
            continue;
        prop[count] = FreeWubiPanelProxyComplexStatus2String(instance, compstatus);
        count ++;
    }

    FcitxUIStatus *status;
    for (status = (FcitxUIStatus *) utarray_front(uistats);
         status != NULL;
         status = (FcitxUIStatus *) utarray_next(uistats, status)) {
        if (!status->visible)
            continue;
        prop[count] = FreeWubiPanelProxyStatus2String(instance, status);
        count ++;
    }

    FreeWubiPanelProxyRegisterProperties(prop, count);

    while (count --)
        free(prop[count]);

    free(prop);
}

void FreeWubiPanelProxySetIMStatus()
{
    FcitxInstance* instance = freeWubiPanel->owner;
    char* status = NULL;

    SetIMIcon(instance, &status);
    FreeWubiPanelProxyUpdateProperty(status);
    free(status);
}

void FreeWubiPanelProxyOnTriggerOff()
{
    FreeWubiPanelProxyEnable(false);
    FreeWubiPanelProxySetIMStatus();
}

void FreeWubiPanelProxyOnTriggerOn()
{
    FreeWubiPanelProxyEnable(true);
    FreeWubiPanelProxySetIMStatus();
}

void FreeWubiPanelProxyCloseInputWindow()
{
    // FcitxLog(DEBUG, "FreeWubiPanelProxyCloseInputWindow");
    /* why freeWubiPanel sucks, there is not obvious method to close it */
    FreeWubiPanelProxyShowAux(false);
    FreeWubiPanelProxyShowPreedit(false);
    FreeWubiPanelProxyShowLookupTable(false);
}

static DBusMessage* createPanelMethodCallMessage(const char* methodName) {
    return dbus_message_new_method_call(FREEWUBI_PANEL_SERVICENAME,
        FREEWUBI_PANEL_OBJECTPATH, FREEWUBI_PANEL_INTERFACE, methodName);
}

void FreeWubiPanelProxySetSpotRect(int x, int y, int w, int h)
{
    if (freeWubiPanel->lastUpdateX == x && freeWubiPanel->lastUpdateY == y && freeWubiPanel->lastUpdateW == w && freeWubiPanel->lastUpdateH == h)
        return;
    freeWubiPanel->lastUpdateX = x;
    freeWubiPanel->lastUpdateY = y;
    freeWubiPanel->lastUpdateW = w;
    freeWubiPanel->lastUpdateH = h;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;

    // create a signal and check for errors
    msg = createPanelMethodCallMessage("SetSpotRect");
    if (NULL == msg) {
        // FcitxLog(DEBUG, "Message Null");
        return;
    }

    if (!dbus_message_append_args(
            msg,
            DBUS_TYPE_INT32, &x,
            DBUS_TYPE_INT32, &y,
            DBUS_TYPE_INT32, &w,
            DBUS_TYPE_INT32, &h,
            DBUS_TYPE_INVALID
            )) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // send the message and flush the connection
    if (!dbus_connection_send(freeWubiPanel->conn, msg, &serial)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // free the message
    dbus_message_unref(msg);
}

void FreeWubiPanelProxySetLookupTable(char *labels[], int nLabel,
                       char *texts[], int nText,
                       boolean has_prev,
                       boolean has_next,
                       int cursor,
                       int layout)
{
    int i;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors
    msg = createPanelMethodCallMessage("SetLookupTable");
    if (NULL == msg) {
        // FcitxLog(DEBUG, "Message Null");
        return;
    }
    for (i = 0; i < nLabel; i++) {
        if (!fcitx_utf8_check_string(labels[i]))
            return;
    }
    for (i = 0; i < nText; i++) {
        if (!fcitx_utf8_check_string(texts[i]))
            return;
    }
    DBusMessageIter subLabel;
    DBusMessageIter subText;
    DBusMessageIter subAttrs;
    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "s", &subLabel);
    for (i = 0; i < nLabel; i++) {
        if (!dbus_message_iter_append_basic(&subLabel, DBUS_TYPE_STRING, &labels[i])) {
            // FcitxLog(DEBUG, "Out Of Memory!");
        }
    }
    dbus_message_iter_close_container(&args, &subLabel);

    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "s", &subText);
    for (i = 0; i < nText; i++) {
        if (!dbus_message_iter_append_basic(&subText, DBUS_TYPE_STRING, &texts[i])) {
            // FcitxLog(DEBUG, "Out Of Memory!");
        }
    }
    dbus_message_iter_close_container(&args, &subText);

    char *attr = "";
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "s", &subAttrs);
    for (i = 0; i < nLabel; i++) {
        if (!dbus_message_iter_append_basic(&subAttrs, DBUS_TYPE_STRING, &attr)) {
            // FcitxLog(DEBUG, "Out Of Memory!");
        }
    }
    dbus_message_iter_close_container(&args, &subAttrs);

    dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &has_prev);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &has_next);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &cursor);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &layout);

    // send the message and flush the connection
    if (!dbus_connection_send(freeWubiPanel->conn, msg, &serial)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // free the message
    dbus_message_unref(msg);
}

void FreeWubiPanelProxyMoveInputWindow()
{
    // FcitxLog(DEBUG, "FreeWubiPanelProxyMoveInputWindow");
    freeWubiPanel->iOffsetX = 12;
    freeWubiPanel->iOffsetY = 0;

    int x = 0, y = 0, w = 0, h = 0;

    FcitxInputContext* ic = FcitxInstanceGetCurrentIC(freeWubiPanel->owner);
    if (!ic) {
        return;
    }
    FcitxInstanceGetWindowRect(freeWubiPanel->owner, ic, &x, &y, &w, &h);

    FreeWubiPanelProxySetSpotRect(x, y, w, h);
}

char* FreeWubiPanelProxyStatus2String(FcitxInstance* instance, FcitxUIStatus* status)
{
    char *result;
    FcitxUIMenu *menu = FcitxUIGetMenuByStatusName(instance, status->name);
    fcitx_utils_alloc_cat_str(result, "/Fcitx/", status->name, ":",
                              status->shortDescription, ":fcitx-", status->name,
                              ((status->getCurrentStatus(status->arg)) ?
                               "-active:" : "-inactive:"),
                              status->longDescription,
                              menu ? ":menu" : ":");
    return result;
}


char* FreeWubiPanelProxyComplexStatus2String(FcitxInstance* instance, FcitxUIComplexStatus* status)
{
    const char* icon = status->getIconName(status->arg);
    char *str;
    boolean result = CheckAddPrefix(&icon);
    FcitxUIMenu *menu = FcitxUIGetMenuByStatusName(instance, status->name);
    fcitx_utils_alloc_cat_str(str, "/Fcitx/", status->name, ":",
                              status->shortDescription,
                              (result ? ":fcitx-" : ":"),
                              icon, ":",
                              status->longDescription,
                              menu ? ":menu" : ":"
                             );
    return str;
}

int FreeWubiUINewMessageToOldStyleMessage(FcitxInputState* input, FcitxMessages* msgUp, FcitxMessages* msgDown)
{
    int i = 0;

    int extraLength = input->iCursorPos;
    FcitxMessagesSetMessageCount(msgUp, 0);
    FcitxMessagesSetMessageCount(msgDown, 0);

    for (i = 0; i < FcitxMessagesGetMessageCount(input->msgAuxUp) ; i ++) {
        FcitxMessagesAddMessageStringsAtLast(msgUp, FcitxMessagesGetMessageType(input->msgAuxUp, i), FcitxMessagesGetMessageString(input->msgAuxUp, i));
        extraLength += strlen(FcitxMessagesGetMessageString(input->msgAuxUp, i));
    }

    for (i = 0; i < FcitxMessagesGetMessageCount(input->msgPreedit) ; i ++)
        FcitxMessagesAddMessageStringsAtLast(msgUp, FcitxMessagesGetMessageType(input->msgPreedit, i), FcitxMessagesGetMessageString(input->msgPreedit, i));

    for (i = 0; i < FcitxMessagesGetMessageCount(input->msgAuxDown) ; i ++)
        FcitxMessagesAddMessageStringsAtLast(msgDown, FcitxMessagesGetMessageType(input->msgAuxDown, i), FcitxMessagesGetMessageString(input->msgAuxDown, i));

    FcitxCandidateWord* candWord = NULL;

    for (candWord = FcitxCandidateWordGetCurrentWindow(input->candList), i = 0;
            candWord != NULL;
            candWord = FcitxCandidateWordGetCurrentWindowNext(input->candList, candWord), i ++) {
        char strTemp[3] = { '\0', '\0', '\0' };
        strTemp[0] = FcitxCandidateWordGetChoose(input->candList)[i];

        // we don't need this
        // if (instance->config->bPointAfterNumber)
            strTemp[1] = '.';

        if (candWord->strWord == NULL)
            continue;

        unsigned int mod = FcitxCandidateWordGetModifier(input->candList);

        FcitxMessagesAddMessageStringsAtLast(
            msgDown, MSG_INDEX, (mod & FcitxKeyState_Super) ? "M-" : "",
            (mod & FcitxKeyState_Ctrl) ? "C-" : "",
            (mod & FcitxKeyState_Alt) ? "A-" : "",
            (mod & FcitxKeyState_Shift) ? "S-" : "", strTemp);

        FcitxMessageType type = candWord->wordType;

        if (i == 0
            && FcitxCandidateWordGetCurrentPage(input->candList) == 0
            && type == MSG_OTHER
            // we don't need this too
            // && FcitxUIUseDefaultHighlight(instance, input->candList)
        ) {
            type = MSG_FIRSTCAND;
        }

        FcitxMessagesAddMessageStringsAtLast(msgDown, type, candWord->strWord);

        if (candWord->strExtra && strlen(candWord->strExtra))
            FcitxMessagesAddMessageStringsAtLast(msgDown, candWord->extraType,
                                                 candWord->strExtra);

        FcitxMessagesAddMessageStringsAtLast(msgDown, MSG_OTHER, " ");
    }

    return extraLength;
}

void FreeWubiPanelProxyShowInputWindow()
{
    FcitxInstance* instance = freeWubiPanel->owner;
    FcitxInputState* input = FreeWubiGetInputState();
    FcitxCandidateWordList* candList = input->candList;
    freeWubiPanel->iCursorPos = FreeWubiUINewMessageToOldStyleMessage(input, freeWubiPanel->messageUp, freeWubiPanel->messageDown);
    FcitxMessages* messageDown = freeWubiPanel->messageDown;
    FcitxMessages* messageUp = freeWubiPanel->messageUp;
    // FcitxLog(DEBUG, "FreeWubiPanelProxyShowInputWindow");
    FreeWubiPanelProxyMoveInputWindow();

    boolean hasPrev = FcitxCandidateWordHasPrev(candList);
    boolean hasNext = FcitxCandidateWordHasNext(candList);
    FcitxCandidateLayoutHint layout = FcitxCandidateWordGetLayoutHint(candList);

    int n = FcitxMessagesGetMessageCount(messageDown);
    int nLabels = 0;
    int nTexts = 0;
    char *label[33];
    char *text[33];
    char cmb[FREEWUBI_PANEL_BUFFER_SIZE] = "";
    int i;
    int pos = -1;

    if (n) {
        for (i = 0; i < n; i++) {
            // FcitxLog(DEBUG, "Type: %d Text: %s" , FcitxMessagesGetMessageType(messageDown, i), FcitxMessagesGetMessageString(messageDown, i));

            if (FcitxMessagesGetMessageType(messageDown, i) == MSG_INDEX) {
                if (nLabels) {
                    text[nTexts++] = strdup(cmb);
                }
                char *needfree = FcitxInstanceProcessOutputFilter(instance, FcitxMessagesGetMessageString(messageDown, i));
                char *msgstr;
                if (needfree)
                    msgstr = needfree;
                else
                    msgstr = strdup(FcitxMessagesGetMessageString(messageDown, i));

                label[nLabels++] = msgstr;
                strcpy(cmb, "");
            } else {
                char *needfree = FcitxInstanceProcessOutputFilter(instance, FcitxMessagesGetMessageString(messageDown, i));
                char *msgstr;
                if (needfree)
                    msgstr = needfree;
                else
                    msgstr = FcitxMessagesGetMessageString(messageDown, i);

                if (strlen(cmb) + strlen(msgstr) + 1 < FREEWUBI_PANEL_BUFFER_SIZE)
                    strcat(cmb, msgstr);
                if (needfree)
                    free(needfree);
                if (FcitxMessagesGetMessageType(messageDown, i) == MSG_FIRSTCAND)
                    pos = nTexts;
            }
        }
        text[nTexts++] = strdup(cmb);
        if (nLabels < nTexts) {
            for (; nLabels < nTexts; nLabels++) {
                label[nLabels] = strdup("");
            }
        } else if (nTexts < nLabels) {
            for (; nTexts < nLabels; nTexts++) {
                text[nTexts] = strdup("");
            }
        }
        // FcitxLog(DEBUG, "Labels %d, Texts %d, CMB:%s", nLabels, nTexts, cmb);
        if (nTexts == 0) {
            FreeWubiPanelProxyShowLookupTable(false);
        } else {
            FreeWubiPanelProxySetLookupTable(label, nLabels, text, nTexts, hasPrev, hasNext, pos, layout);
            FreeWubiPanelProxyShowLookupTable(true);
        }
        for (i = 0; i < nTexts; i++)
            free(text[i]);
        for (i = 0; i < nLabels; i++)
            free(label[i]);
    } else {
        FreeWubiPanelProxySetLookupTable(NULL, 0, NULL, 0, hasNext, hasNext, pos, layout);
        FreeWubiPanelProxyShowLookupTable(false);
    }

    n = FcitxMessagesGetMessageCount(messageUp);
    char aux[MESSAGE_MAX_LENGTH] = "";
    char empty[MESSAGE_MAX_LENGTH] = "";
    if (n) {
        for (i = 0; i < n; i++) {

            char *needfree = FcitxInstanceProcessOutputFilter(instance, FcitxMessagesGetMessageString(messageUp, i));
            char *msgstr;
            if (needfree)
                msgstr = needfree;
            else
                msgstr = FcitxMessagesGetMessageString(messageUp, i);

            strcat(aux, msgstr);
            if (needfree)
                free(needfree);
            // FcitxLog(DEBUG, "updateMesssages Up:%s", aux);
        }

        if (FcitxInputStateGetShowCursor(input)) {
            FreeWubiPanelProxyUpdatePreeditText(aux);
            FreeWubiPanelProxyUpdateAux(empty);
            FreeWubiPanelProxyShowPreedit(true);
            FreeWubiPanelProxyUpdatePreeditCaret(FreeWubiPanelProxyCalCursorPos());
            FreeWubiPanelProxyShowAux(false);
        } else {
            FreeWubiPanelProxyUpdatePreeditText(empty);
            FreeWubiPanelProxyUpdateAux(aux);
            FreeWubiPanelProxyShowPreedit(false);
            FreeWubiPanelProxyShowAux(true);
        }
    } else {
        FreeWubiPanelProxyShowPreedit(false);
        FreeWubiPanelProxyShowAux(false);
    }
}

static DBusHandlerResult
FreeWubiPanelProxyDBusEventHandler(DBusConnection *connection,
                         DBusMessage *msg, void *arg)
{
    FCITX_UNUSED(connection);
    FreeWubiPanelProxy* freeWubiPanel = (FreeWubiPanelProxy*)arg;

    if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {
        DBusMessage *reply = dbus_message_new_method_return(msg);

        dbus_message_append_args(reply, DBUS_TYPE_STRING, &freewubipanel_introspection_xml, DBUS_TYPE_INVALID);
        dbus_connection_send(freeWubiPanel->conn, reply, NULL);
        dbus_message_unref(reply);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void FreeWubiPanelProxyReset()
{
    freeWubiPanel->lastCursor = -2;
    freeWubiPanel->lastUpdateH = -2;
    freeWubiPanel->lastUpdateW = -2;
    freeWubiPanel->lastUpdateX = -2;
    freeWubiPanel->lastUpdateY = -2;
}

DBusHandlerResult FreeWubiPanelProxyDBusFilter(DBusConnection* connection, DBusMessage* msg, void* user_data)
{
    FCITX_UNUSED(connection);
    FreeWubiPanelProxy* freeWubiPanel = (FreeWubiPanelProxy*) user_data;
    FcitxInstance* instance = freeWubiPanel->owner;
    FcitxInputState* input = FreeWubiGetInputState();
    int int0;
    const char* s0 = NULL;
    if (dbus_message_is_signal(msg, FREEWUBI_PANEL_INTERFACE, "PanelCreated2")) {
        FreeWubiPanelProxyReset();
        FreeWubiPanelProxyRegisterAllStatus();
        return DBUS_HANDLER_RESULT_HANDLED;
    } else if (dbus_message_is_signal(msg, FREEWUBI_PANEL_INTERFACE, "SelectCandidate")) {
        DBusError error;
        dbus_error_init(&error);
        if (dbus_message_get_args(msg, &error, DBUS_TYPE_INT32, &int0 , DBUS_TYPE_INVALID)) {
            FcitxCandidateWordList *candList = FreeWubiInputStateGetCandidateList(input);
            if (IRV_COMMIT_STRING == FcitxCandidateWordChooseByIndex(candList, int0)) {
                FcitxInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), FcitxInputStateGetOutputString(input));
                FreeWubiResetInputState(input);
                FreeWubiPanelProxyCloseInputWindow();
            }
        }
        dbus_error_free(&error);
        return DBUS_HANDLER_RESULT_HANDLED;
    } else if (dbus_message_is_signal(msg, FREEWUBI_PANEL_INTERFACE, "LookupTablePageUp")) {
        FcitxLog(DEBUG, "LookupTablePageUp");
        FcitxCandidateWordList *candList = FreeWubiInputStateGetCandidateList(input);
        if (FcitxCandidateWordPageCount(candList) != 0) {
            FcitxCandidateWordGoPrevPage(candList);
            FreeWubiPanelProxyShowInputWindow();
        }
        return DBUS_HANDLER_RESULT_HANDLED;
    } else if (dbus_message_is_signal(msg, FREEWUBI_PANEL_INTERFACE, "LookupTablePageDown")) {
        FcitxLog(DEBUG, "LookupTablePageDown");
        FcitxCandidateWordList *candList = FreeWubiInputStateGetCandidateList(input);
        if (FcitxCandidateWordPageCount(candList) != 0) {
            FcitxCandidateWordGoNextPage(candList);
            FreeWubiPanelProxyShowInputWindow();
        }
        return DBUS_HANDLER_RESULT_HANDLED;
    } else if (dbus_message_is_signal(msg, FREEWUBI_PANEL_INTERFACE, "ReloadConfig")) {
        FcitxLog(DEBUG, "ReloadConfig");
        FcitxInstanceReloadConfig(instance);
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusMessage* createInputMethodSignalMessage(const char* signalName) {
    return dbus_message_new_signal(FREEWUBI_INPUTMETHOD_OBJECTPATH,
                                   FREEWUBI_INPUTMETHOD_SERVICENAME,
                                   signalName);
}

void FreeWubiPanelProxyRegisterProperties(char *props[], int n)
{
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;
    int i;

    // create a signal and check for errors
    msg = createInputMethodSignalMessage("RegisterProperties");
    if (NULL == msg) {
        // FcitxLog(DEBUG, "Message Null");
        return;
    }

    for (i = 0; i < n; i++) {
        if (!fcitx_utf8_check_string(props[i]))
            return;
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    DBusMessageIter sub;
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "s", &sub);
    for (i = 0; i < n; i++) {
        if (!dbus_message_iter_append_basic(&sub, DBUS_TYPE_STRING, &props[i])) {
            // FcitxLog(DEBUG, "Out Of Memory!");
        }
    }
    dbus_message_iter_close_container(&args, &sub);

    // send the message and flush the connection
    if (!dbus_connection_send(freeWubiPanel->conn, msg, &serial)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // free the message
    dbus_message_unref(msg);

}

void FreeWubiPanelProxyUpdateProperty(char *prop)
{
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors
    msg = createInputMethodSignalMessage("UpdateProperty");
    if (NULL == msg) {
        // FcitxLog(DEBUG, "Message Null");
        return;
    }

    if (!fcitx_utf8_check_string(prop))
        return;

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // send the message and flush the connection
    if (!dbus_connection_send(freeWubiPanel->conn, msg, &serial)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // free the message
    dbus_message_unref(msg);

}

void FreeWubiPanelProxyEnable(boolean toEnable)
{
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors
    msg = createInputMethodSignalMessage("Enable");
    if (NULL == msg) {
        // FcitxLog(DEBUG, "Message Null");
        return;
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &toEnable)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // send the message and flush the connection
    if (!dbus_connection_send(freeWubiPanel->conn, msg, &serial)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // free the message
    dbus_message_unref(msg);
}

void FreeWubiPanelProxyShowAux(boolean toShow)
{
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors
    msg = createInputMethodSignalMessage("ShowAux");
    if (NULL == msg) {
        // FcitxLog(DEBUG, "Message Null");
        return;
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &toShow)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // send the message and flush the connection
    if (!dbus_connection_send(freeWubiPanel->conn, msg, &serial)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // free the message
    dbus_message_unref(msg);

}

void FreeWubiPanelProxyShowPreedit(boolean toShow)
{
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors
    msg = createInputMethodSignalMessage("ShowPreedit");
    if (NULL == msg) {
        // FcitxLog(DEBUG, "Message Null");
        return;
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &toShow)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // send the message and flush the connection
    if (!dbus_connection_send(freeWubiPanel->conn, msg, &serial)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // free the message
    dbus_message_unref(msg);

}

void FreeWubiPanelProxyShowLookupTable(boolean toShow)
{
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors
    msg = createInputMethodSignalMessage("ShowLookupTable");
    if (NULL == msg) {
        // FcitxLog(DEBUG, "Message Null");
        return;
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &toShow)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // send the message and flush the connection
    if (!dbus_connection_send(freeWubiPanel->conn, msg, &serial)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // free the message
    dbus_message_unref(msg);

}

void FreeWubiPanelProxyUpdatePreeditCaret(int position)
{
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors
    msg = createInputMethodSignalMessage("UpdatePreeditCaret");
    if (NULL == msg) {
        // FcitxLog(DEBUG, "Message Null");
        return;
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &position)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // send the message and flush the connection
    if (!dbus_connection_send(freeWubiPanel->conn, msg, &serial)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // free the message
    dbus_message_unref(msg);

}

void FreeWubiPanelProxyUpdatePreeditText(char *text)
{
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors
    msg = createInputMethodSignalMessage("UpdatePreeditText");
    if (NULL == msg) {
        // FcitxLog(DEBUG, "Message Null");
        return;
    }

    char *attr = "";
    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &text)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &attr)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // send the message and flush the connection
    if (!dbus_connection_send(freeWubiPanel->conn, msg, &serial)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // free the message
    dbus_message_unref(msg);

}

void FreeWubiPanelProxyUpdateAux(char *text)
{
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors
    msg = createInputMethodSignalMessage("UpdateAux");
    if (NULL == msg) {
        // FcitxLog(DEBUG, "Message Null");
        return;
    }
    if (!fcitx_utf8_check_string(text))
        return;

    char *attr = "";

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &text)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &attr)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // send the message and flush the connection
    if (!dbus_connection_send(freeWubiPanel->conn, msg, &serial)) {
        // FcitxLog(DEBUG, "Out Of Memory!");
    }

    // free the message
    dbus_message_unref(msg);

}

int FreeWubiPanelProxyCalCursorPos()
{
    int i = 0;
    unsigned int iChar;
    int iCount = 0;
    FcitxMessages *messageUp = freeWubiPanel->messageUp;

    const char *p1;
    const char *pivot;

    iChar = freeWubiPanel->iCursorPos;

    for (i = 0;i < FcitxMessagesGetMessageCount(messageUp);i++) {
        if (freeWubiPanel->iCursorPos && iChar) {
            p1 = pivot = FcitxMessagesGetMessageString(messageUp, i);
            while (*p1 && p1 < pivot + iChar) {
                p1 = p1 + fcitx_utf8_char_len(p1);
                iCount ++;
            }
            if (strlen(FcitxMessagesGetMessageString(messageUp, i)) > iChar) {
                iChar = 0;
            } else {
                iChar -= strlen(FcitxMessagesGetMessageString(messageUp, i));
            }
        }
    }

    return iCount;
}

void FreeWubiPanelProxyDestroyInstance()
{
    FreeWubiPanelProxyOnTriggerOff();

    FreeWubiPanelProxyCloseInputWindow();
    FreeWubiPanelProxyRegisterProperties(NULL, 0);
    dbus_connection_unregister_object_path(freeWubiPanel->conn, FREEWUBI_INPUTMETHOD_OBJECTPATH);
    dbus_connection_remove_filter(freeWubiPanel->conn, FreeWubiPanelProxyDBusFilter, freeWubiPanel);

    dbus_bus_remove_match(freeWubiPanel->conn,
                          "type='signal',sender='"FREEWUBI_PANEL_SERVICENAME"',interface='"FREEWUBI_PANEL_INTERFACE"'",
                          NULL);

    dbus_connection_flush(freeWubiPanel->conn);

    free(freeWubiPanel->messageUp);
    free(freeWubiPanel->messageDown);
    free(freeWubiPanel);

    freeWubiPanel = NULL;
}

// // kate: indent-mode cstyle; space-indent on; indent-width 0;