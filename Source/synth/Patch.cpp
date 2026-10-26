#include "Patch.h"

namespace dmb
{
namespace
{
    const ParamInfo layerTable[] = {
#define DMB_X(name, id, mn, mx, def, isInt) { id, mn, mx, def, isInt },
        DMB_LAYER_PARAMS (DMB_X)
#undef DMB_X
    };
    const ParamInfo globalTable[] = {
#define DMB_X(name, id, mn, mx, def, isInt) { id, mn, mx, def, isInt },
        DMB_GLOBAL_PARAMS (DMB_X)
#undef DMB_X
    };
}

const ParamInfo& layerInfo (int index)  { return layerTable[index]; }
const ParamInfo& globalInfo (int index) { return globalTable[index]; }

Layer::Layer()
{
    for (int k = 0; k < lp::count; ++k) v[(size_t) k] = layerTable[k].def;
}

Patch::Patch()
{
    for (int k = 0; k < gp::count; ++k) g[(size_t) k] = globalTable[k].def;
    layer[0][lp::engine] = (float) Engine::analog;
    layer[1][lp::engine] = (float) Engine::off;
}

void Patch::clampAll()
{
    for (auto& l : layer)
        for (int k = 0; k < lp::count; ++k)
        {
            const auto& inf = layerTable[k];
            auto& x = l.v[(size_t) k];
            if (! std::isfinite (x)) x = inf.def;
            x = juce::jlimit (inf.min, inf.max, x);
            if (inf.isInt) x = (float) std::lround (x);
        }
    for (int k = 0; k < gp::count; ++k)
    {
        const auto& inf = globalTable[k];
        auto& x = g[(size_t) k];
        if (! std::isfinite (x)) x = inf.def;
        x = juce::jlimit (inf.min, inf.max, x);
        if (inf.isInt) x = (float) std::lround (x);
    }
}

juce::ValueTree Patch::toTree() const
{
    juce::ValueTree t ("PATCH");
    t.setProperty ("v", 1, nullptr);
    for (int k = 0; k < gp::count; ++k)
        t.setProperty (globalTable[k].id, g[(size_t) k], nullptr);
    for (int li = 0; li < 2; ++li)
    {
        juce::ValueTree l ("LAYER");
        for (int k = 0; k < lp::count; ++k)
            l.setProperty (layerTable[k].id, layer[(size_t) li].v[(size_t) k], nullptr);
        t.appendChild (l, nullptr);
    }
    return t;
}

Patch Patch::fromTree (const juce::ValueTree& t)
{
    Patch p;
    if (! t.isValid()) return p;
    for (int k = 0; k < gp::count; ++k)
        p.g[(size_t) k] = (float) t.getProperty (globalTable[k].id, globalTable[k].def);
    int li = 0;
    for (auto l : t)
    {
        if (! l.hasType ("LAYER") || li > 1) continue;
        for (int k = 0; k < lp::count; ++k)
            p.layer[(size_t) li].v[(size_t) k] = (float) l.getProperty (layerTable[k].id, layerTable[k].def);
        ++li;
    }
    p.clampAll();
    return p;
}

float delayDivisionBeats (int index)
{
    static const float beats[] = { 0.25f, 0.375f, 1.0f / 3.0f, 0.5f, 2.0f / 3.0f, 0.75f, 1.0f, 1.5f, 2.0f, 3.0f };
    return beats[juce::jlimit (0, 9, index)];
}

juce::String delayDivisionName (int index)
{
    static const char* names[] = { "1/16", "1/16D", "1/8T", "1/8", "1/4T", "1/8D", "1/4", "1/4D", "1/2", "1/2D" };
    return names[juce::jlimit (0, 9, index)];
}
} // namespace dmb
