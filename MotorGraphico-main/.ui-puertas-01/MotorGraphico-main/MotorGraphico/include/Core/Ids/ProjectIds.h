#pragma once

#include "Core/Ids/StrongId.h"

namespace ids {

struct ProjectIdTag {
    static constexpr const char* kNamespace = "project";
};
struct DocumentIdTag {
    static constexpr const char* kNamespace = "document";
};
struct MapIdTag {
    static constexpr const char* kNamespace = "map";
};
struct LayerIdTag {
    static constexpr const char* kNamespace = "layer";
};
struct AssetIdTag {
    static constexpr const char* kNamespace = "asset";
};
struct EntityIdTag {
    static constexpr const char* kNamespace = "entity";
};
struct RegionIdTag {
    static constexpr const char* kNamespace = "region";
};
struct EventIdTag {
    static constexpr const char* kNamespace = "event";
};

using ProjectId = StrongId<ProjectIdTag>;
using DocumentId = StrongId<DocumentIdTag>;
using MapId = StrongId<MapIdTag>;
using LayerId = StrongId<LayerIdTag>;
using AssetId = StrongId<AssetIdTag>;
using EntityId = StrongId<EntityIdTag>;
using RegionId = StrongId<RegionIdTag>;
using EventId = StrongId<EventIdTag>;

}  // namespace ids
