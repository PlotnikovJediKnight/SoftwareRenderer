#ifndef PV_MODELSTATUS_H
#define PV_MODELSTATUS_H


namespace pv {

    enum class MeshStatus { NO_MODEL, TRIANGLES_ONLY, CONVERTIBLE_TO_TRIANGLES, NON_CONVERTIBLE_TO_TRIANGLES };

    enum class NormalStatus { NO_MODEL, NORMALS_PROVIDED, NO_NORMALS_PROVIDED };

    enum class TextureStatus {  NO_MODEL, TEXTURE_COORD_PROVIDED, NO_TEXTURE_COORD_PROVIDED };

} // namespace pv

#endif // PV_MODELSTATUS_H
