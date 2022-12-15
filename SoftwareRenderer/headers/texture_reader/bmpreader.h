#ifndef BMPREADER_H
#define BMPREADER_H

#include "headers/texture_reader/texturereader.h"

namespace pv {

    class BMPReader : public TextureReader {
    public:
        virtual TextureHolder GetTexture(QString path, TEXTURE_COLOR_MODEL textColorModel) const override;
    };

}

#endif // BMPREADER_H
