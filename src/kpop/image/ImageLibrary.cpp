#include "ImageLibrary.hpp"

#include <utility>

namespace kpop::image
{
    ImageLibrary::ImageLibrary(std::filesystem::path directory, const int maxDimension) :
        m_store(std::move(directory)),
        m_maxDimension(maxDimension)
    {}

    domain::ImageId ImageLibrary::importFile(const std::filesystem::path& sourceFile)
    {
        return m_store.add(prepareImage(sourceFile, m_maxDimension));
    }

    std::optional<std::filesystem::path> ImageLibrary::pathOf(const domain::ImageId& id) const
    {
        if (!m_store.contains(id))
            return std::nullopt;

        return m_store.pathOf(id);
    }

    ImageStore& ImageLibrary::store()
    {
        return m_store;
    }

    const ImageStore& ImageLibrary::store() const
    {
        return m_store;
    }
}
