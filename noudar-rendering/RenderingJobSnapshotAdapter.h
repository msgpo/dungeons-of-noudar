//
// Created by monty on 07/02/17.
//

#ifndef DUNGEONSOFNOUDAR_NDK_RENDERINGJOBSNAPSHOTADAPTER_H
#define DUNGEONSOFNOUDAR_NDK_RENDERINGJOBSNAPSHOTADAPTER_H

namespace odb {
	const static bool kFogEnabled = false;
	const static VBORegisterId mNullVBO = "null";
	const static TextureName mSkyBoxTextureName = "sky";
	const static TextureName mNullTexture = "null";

	static const long kAnimationLength = 500;
	const static int kSkyTextureLength = 400;

	using Shade = float;

	using TextureId = int;

	class RenderingJobSnapshotAdapter {

		static glm::mat4 getSkyTransform(long offset);

		static glm::mat4 getFloorTransform(glm::vec3 translation);

		static glm::mat4 getCornerLeftFarTransform(glm::vec3 translation);

		static glm::mat4 getCornerLeftNearTransform(glm::vec3 translation);

		static glm::mat4 getCubeTransform(glm::vec3 translation, int scale);

	public:
		static void
		readSnapshot(const NoudarDungeonSnapshot &snapshot, std::unordered_map<ETextures, std::vector<VBORenderingJob>> &batches,
		             const CTilePropertyMap &tileProperties,
		             const std::unordered_map<VBORegisterId, VBORegister> &VBORegisters,
		             const std::unordered_map<std::string, ETextures> &textureRegistry
		);

		static glm::vec2
		easingAnimationCurveStep(glm::vec2 prevPosition, glm::vec2 destPosition, long animationTime, long timestamp
		                         );
	};
}

#endif //DUNGEONSOFNOUDAR_NDK_RENDERINGJOBSNAPSHOTADAPTER_H
