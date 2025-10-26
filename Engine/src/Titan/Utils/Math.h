#pragma once

#include "Titan/PCH.h"

namespace Titan::Math {

	bool TI_API DecomposeTransform(const glm::mat4& transform, glm::vec3& outTranslation, glm::vec3& outRotation, glm::vec3& outScale);

}