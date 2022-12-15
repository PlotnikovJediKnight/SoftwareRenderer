#ifndef TEXTUREREADER_H
#define TEXTUREREADER_H

#include <QString>
#include "headers/texturing/texture.h"

namespace pv {

    using TextureHolder = std::unique_ptr<pv::Texture>;

    class TextureReader{
    public:

        virtual TextureHolder GetTexture(QString path, TEXTURE_COLOR_MODEL textColorModel) const = 0;
    };

}

#endif // TEXTUREREADER_H
