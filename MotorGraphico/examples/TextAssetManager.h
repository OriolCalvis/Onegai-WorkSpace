#pragma once

#include "Core/Resources/ResourceManager.h"
#include "TextAsset.h"

class TextAssetManager : public ResourceManager<TextAsset> {
protected:
    std::unique_ptr<TextAsset> loadFromDisk(const std::string& path) override;
};
