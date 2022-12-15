#include "headers/texture_reader/bmpreader.h"

#include <QFile>
#include <QTextStream>
#include <iostream>

using namespace std;

namespace pv {

    size_t GetColorModelBPP(TEXTURE_COLOR_MODEL textColorModel){
        switch (textColorModel){
            case TEXTURE_COLOR_MODEL::RGB24: return 24;
            case TEXTURE_COLOR_MODEL::MONO8: return 8;
        }
        throw runtime_error("Wrong Texture Color Model!");
    }

    TextureHolder BMPReader::GetTexture(QString path, TEXTURE_COLOR_MODEL textColorModel) const {
        QFile inputFile(path);
        inputFile.open(QIODevice::ReadOnly);
        QTextStream in(&inputFile);

        const QByteArray allBytes = inputFile.readAll();

        constexpr size_t
                BITMAP_WIDTH_OFFSET = 18,
                BITMAP_HEIGHT_OFFSET = 22,
                BPP_OFFSET = 28;

        assert(sizeof(unsigned short int) == 2);
        using USI = unsigned short int;

        assert(sizeof(signed int) == 4);
        using SIGNED_INT = signed int;

        const SIGNED_INT *widthPointer = reinterpret_cast<const SIGNED_INT*>(allBytes.data() + BITMAP_WIDTH_OFFSET);
        const SIGNED_INT *heightPointer = reinterpret_cast<const SIGNED_INT*>(allBytes.data() + BITMAP_HEIGHT_OFFSET);
        const USI *bppPointer = reinterpret_cast<const USI*>(allBytes.data() + BPP_OFFSET);

        cerr << "Image is " << *widthPointer << "x" << *heightPointer << " with " << *bppPointer << " bpp" << endl;

        const size_t bpp = GetColorModelBPP(textColorModel);
        assert(*bppPointer == bpp);

        SIGNED_INT width = *widthPointer;
        SIGNED_INT height = *heightPointer;

        size_t textureWidth = (width > 0) ? width : 0;
        size_t textureHeight = (height > 0) ? height : 0;

        auto textureHolder = make_unique<Texture>(textureWidth, textureHeight, textColorModel);

        assert(sizeof(int) == 4);
        constexpr size_t PIXEL_DATA_START_ADDR_OFFSET = 10;
        const int *pixelDataStartAddrPointer = reinterpret_cast<const int*>(allBytes.data() + PIXEL_DATA_START_ADDR_OFFSET);

        const size_t RAW_PIXEL_DATA_OFFSET = *pixelDataStartAddrPointer;

        const size_t COMPONENTS_COUNT = bpp / 8;
        const uchar* rawBmpPixelData = reinterpret_cast<const uchar*>(allBytes.data() + RAW_PIXEL_DATA_OFFSET);

        //
        uchar* rawPixelData = new uchar[textureWidth * textureHeight * COMPONENTS_COUNT];
        //

        const uchar* bmpSrc = rawBmpPixelData;
        uchar* pixelDataDest = rawPixelData;

        const size_t RELEVANT_BYTES_PER_ROW = textureWidth * COMPONENTS_COUNT;
        size_t FULL_BYTES_PER_ROW = RELEVANT_BYTES_PER_ROW;
        while (FULL_BYTES_PER_ROW % 4 != 0) FULL_BYTES_PER_ROW++;

        const size_t PADDING_BYTES_UP_TO_4 = FULL_BYTES_PER_ROW - RELEVANT_BYTES_PER_ROW;

        for (size_t row = 0; row < textureHeight; ++row) {
            std::memcpy(pixelDataDest, bmpSrc, RELEVANT_BYTES_PER_ROW);

            bmpSrc += RELEVANT_BYTES_PER_ROW;
            bmpSrc += PADDING_BYTES_UP_TO_4;

            pixelDataDest += RELEVANT_BYTES_PER_ROW;
        }

        textureHolder->SaveTexture(rawPixelData);

        delete[] rawPixelData;

        //#define TEXTURE_DEBUG
        #ifdef TEXTURE_DEBUG
        for (size_t j = 0; j < textureHolder->GetHeight(); ++j){
            for (size_t i = 0; i < textureHolder->GetWidth(); ++i){
                auto texel = textureHolder->GetTexel(i, j);
                for (auto& t : texel){
                    cout << static_cast<int>(t) << " ";
                }
                cout << "|";
            }
            cout << endl;
        }
        #endif

        return textureHolder;
    }

}
