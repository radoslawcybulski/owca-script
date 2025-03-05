#include "stdafx.h"
#include "line.h"
#include "impl_base.h"

namespace OwcaScript::Internal {
    void Line::serialize_object(Serializer &ser) const {
        ser.serialize(line);
    }
    void Line::deserialize_object(Deserializer &ser) {
        ser.deserialize(line);
    }

}