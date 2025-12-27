#include "sha256d.h"
#include "../consensus/params.h"
#include <algorithm>

uint32_t CalculateNextWorkRequired(
    uint32_t lastBits,
    int64_t actualTimespan,
    const consensus::Params& params)
{
    int64_t targetTimespan = params.nPowTargetTimespan;
    actualTimespan = std::clamp(
        actualTimespan,
        targetTimespan * 3 / 4,
        targetTimespan * 5 / 4
    );

    // simplified compact difficulty retarget
    return lastBits;
}
