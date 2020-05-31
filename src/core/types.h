#pragma once
#include <cstddef>
#include <cstdint>

/* Type definitions */
typedef uint64_t channel_t;
typedef uint32_t doid_t;
typedef uint32_t zone_t;

/* Type limits */
const channel_t CHANNEL_MAX = (channel_t)(-1);
const doid_t DOID_MAX = (doid_t)(-1);
const zone_t ZONE_MAX = (zone_t)(-1);
const size_t ZONE_BITS = sizeof(zone_t) * 8;
const doid_t INDEX_END = (doid_t) - 1;
const doid_t INDEX_ALLOCATED = (doid_t) - 2;

/* DoId constants */
const doid_t INVALID_DO_ID = 0;

/* Channel constants */
const channel_t INVALID_CHANNEL = 0;
const channel_t CONTROL_MESSAGE = 1;
const channel_t BCHAN_CLIENTS = 10;
const channel_t BCHAN_STATESERVERS = 12;
const channel_t BCHAN_DBSERVERS = 13;
const channel_t PARENT_PREFIX = (channel_t(1) << ZONE_BITS);
const channel_t DATABASE_PREFIX = (channel_t(2) << ZONE_BITS);

/* Channel building methods */
inline channel_t location_as_channel(doid_t parent, zone_t zone)
{
    return (channel_t(parent) << ZONE_BITS) | channel_t(zone);
}
inline channel_t object_as_channel(doid_t parent, doid_t child)
{
    return (channel_t(parent) << ZONE_BITS) | channel_t(child);
}
inline channel_t parent_to_children(doid_t parent)
{
    return PARENT_PREFIX | channel_t(parent);
}
inline channel_t database_to_object(doid_t object)
{
    return DATABASE_PREFIX | channel_t(object);
}
