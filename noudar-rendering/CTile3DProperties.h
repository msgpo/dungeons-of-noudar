//
// Created by monty on 14/01/17.
//

#ifndef DUNGEONSOFNOUDAR_NDK_TILEPROPERTIES_H
#define DUNGEONSOFNOUDAR_NDK_TILEPROPERTIES_H
namespace odb {

    class CTile3DProperties;

    using CTileId = int;

    using CTilePropertyMap = std::map< CTileId, CTile3DProperties >;

    using TextureName = std::string;

    class CTile3DProperties {
    public:
        bool mNeedsAlphaTest = true;
        TextureName mCeilingTexture;
        TextureName mFloorTexture;
        TextureName mMainWallTexture;
        VBORegisterId mVBOToRender;
        TextureName mCeilingRepeatedWallTexture;
        TextureName mFloorRepeatedWallTexture;
        int mCeilingRepetitions;
        int mFloorRepetitions;
        float mCeilingHeight;
        float mFloorHeight;

        static CTilePropertyMap parsePropertyList( std::string propertyFile );
    };
}


#endif //DUNGEONSOFNOUDAR_NDK_TILEPROPERTIES_H
