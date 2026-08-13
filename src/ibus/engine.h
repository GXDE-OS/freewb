#ifndef FREEWB_IBUS_ENGINE_H
#define FREEWB_IBUS_ENGINE_H

#include <ibus.h>

#define FREEWB_TYPE_ENGINE (freewb_engine_get_type())
#define FREEWB_ENGINE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), FREEWB_TYPE_ENGINE, FreewbEngine))

typedef struct _FreewbEngine FreewbEngine;
typedef struct _FreewbEngineClass FreewbEngineClass;

GType freewb_engine_get_type();

#endif
