#include "core/AsciiStarMap.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace AstroMap {

static float lerp(float v, float srcLo, float srcHi,
                  float dstLo, float dstHi) {
    if (srcHi == srcLo) return (dstLo + dstHi) * 0.5f;
    return dstLo + (v - srcLo) / (srcHi - srcLo) * (dstHi - dstLo);
}

AsciiStarMap::AsciiStarMap(const StarMapConfig& cfg)
    : m_cfg(cfg) {}

void AsciiStarMap::loadFromEngine(const GalacticEngine& engine) {
    m_stars = engine.getStars();
    computeBounds(m_minA, m_maxA, m_minB, m_maxB);

    float halfA = (m_maxA - m_minA) * 0.5f / m_cfg.zoom;
    float halfB = (m_maxB - m_minB) * 0.5f / m_cfg.zoom;

    const float MIN_HALF = 10.0f;
    halfA = std::max(halfA, MIN_HALF);
    halfB = std::max(halfB, MIN_HALF);

    // Ensure at least 1 world-unit per row/col to avoid duplicate axis labels
    float minHalfForRows = (float)m_cfg.height * 0.6f;
    float minHalfForCols = (float)m_cfg.width  * 0.6f;
    halfB = std::max(halfB, minHalfForRows);
    halfA = std::max(halfA, minHalfForCols);

    float midA = m_cfg.centerX;
    float midB = m_cfg.centerY;

    if (midA == 0.0f && midB == 0.0f) {
        midA = (m_minA + m_maxA) * 0.5f;
        midB = (m_minB + m_maxB) * 0.5f;
    }

    m_minA = midA - halfA;  m_maxA = midA + halfA;
    m_minB = midB - halfB;  m_maxB = midB + halfB;
}

void AsciiStarMap::computeBounds(float& minA, float& maxA,
                                  float& minB, float& maxB) const {
    if (m_stars.empty()) { minA=maxA=minB=maxB=0; return; }

    minA = maxA = minB = maxB = 0;
    for (const auto& s : m_stars) {
        float a, b;
        switch (m_cfg.projection) {
            case Projection::XY: a=(float)s.position.x; b=(float)s.position.y; break;
            case Projection::XZ: a=(float)s.position.x; b=(float)s.position.z; break;
            case Projection::YZ: a=(float)s.position.y; b=(float)s.position.z; break;
        }
        // Clamp DURING iteration so out-of-range stars don't expand bounds
        if (m_cfg.maxRangeLY > 0.0f) {
            if (a < -m_cfg.maxRangeLY || a > m_cfg.maxRangeLY) continue;
            if (b < -m_cfg.maxRangeLY || b > m_cfg.maxRangeLY) continue;
        }
        minA = std::min(minA, a);  maxA = std::max(maxA, a);
        minB = std::min(minB, b);  maxB = std::max(maxB, b);
    }

    float padA = (maxA - minA) * 0.10f + 1.0f;
    float padB = (maxB - minB) * 0.10f + 1.0f;
    minA -= padA;  maxA += padA;
    minB -= padB;  maxB += padB;

    // Secondary clamp on padded bounds
    if (m_cfg.maxRangeLY > 0.0f) {
        minA = std::max(minA, -m_cfg.maxRangeLY);
        maxA = std::min(maxA,  m_cfg.maxRangeLY);
        minB = std::max(minB, -m_cfg.maxRangeLY);
        maxB = std::min(maxB,  m_cfg.maxRangeLY);
    }
}

char AsciiStarMap::glyphForMag(float mag) {
    if (mag < -0.5f) return '*';
    if (mag <  1.0f) return 'o';
    if (mag <  3.0f) return '+';
    return '.';
}

bool AsciiStarMap::project(const Star& s, int& cx, int& cy) const {
    float a, b;
    switch (m_cfg.projection) {
        case Projection::XY: a=(float)s.position.x; b=(float)s.position.y; break;
        case Projection::XZ: a=(float)s.position.x; b=(float)s.position.z; break;
        case Projection::YZ: a=(float)s.position.y; b=(float)s.position.z; break;
    }

    float fx = lerp(a, m_minA, m_maxA, 0.0f, (float)(m_cfg.width  - 1));
    float fy = lerp(b, m_minB, m_maxB, (float)(m_cfg.height - 1), 0.0f);

    cx = (int)std::round(fx);
    cy = (int)std::round(fy);

    // 1-cell margin from left/right edges so labels never bleed into border
    return cx >= 1 && cx < (m_cfg.width - 1) &&
           cy >= 0 && cy < m_cfg.height;
}

std::vector<std::string> AsciiStarMap::makeCanvas() const {
    std::vector<std::string> canvas(m_cfg.height,
                                    std::string(m_cfg.width, ' '));

    if (!m_cfg.showGrid) return canvas;

    int ox, oy;
    {
        float fx = lerp(0.0f, m_minA, m_maxA, 0.0f, (float)(m_cfg.width  - 1));
        float fy = lerp(0.0f, m_minB, m_maxB, (float)(m_cfg.height - 1), 0.0f);
        ox = (int)std::round(fx);
        oy = (int)std::round(fy);
    }

    if (oy >= 0 && oy < m_cfg.height)
        for (int c = 0; c < m_cfg.width; ++c)
            canvas[oy][c] = '-';

    if (ox >= 0 && ox < m_cfg.width)
        for (int r = 0; r < m_cfg.height; ++r)
            canvas[r][ox] = '|';

    if (oy >= 0 && oy < m_cfg.height &&
        ox >= 0 && ox < m_cfg.width)
        canvas[oy][ox] = '+';

    return canvas;
}

std::string AsciiStarMap::render() const {
    const char* axisA = "?", *axisB = "?";
    switch (m_cfg.projection) {
        case Projection::XY: axisA = "X (ly)"; axisB = "Y (ly)"; break;
        case Projection::XZ: axisA = "X (ly)"; axisB = "Z (ly)"; break;
        case Projection::YZ: axisA = "Y (ly)"; axisB = "Z (ly)"; break;
    }

    std::ostringstream out;

    const std::string title = "  ASTRO-MAP  [ " + std::string(axisA) +
                               "  vs  " + axisB + " ]  ";
    const std::string border(m_cfg.width + 4, '=');
    out << '\n' << border << '\n';
    out << "  " << std::left << std::setw(m_cfg.width) << title << "  \n";
    out << border << '\n';

    auto canvas = makeCanvas();

    // ── Pass 1: place all glyphs — glyphs always win over grid chars ──────
    for (const auto& s : m_stars) {
        int cx, cy;
        if (!project(s, cx, cy)) continue;
        canvas[cy][cx] = glyphForMag(s.magnitude);
    }

    // ── Pass 2: place labels — never overwrite any star glyph ─────────────
    if (m_cfg.showLabels) {
        for (const auto& s : m_stars) {
            int cx, cy;
            if (!project(s, cx, cy)) continue;          // ← must call project()
            std::string lbl = s.name.substr(0, 4);
            int lx = cx + 1;
            for (int i = 0; i < (int)lbl.size() && (lx + i) < m_cfg.width; ++i) {
                char existing = canvas[cy][lx + i];
                bool isStarGlyph = (existing == '*' || existing == 'o' ||
                                     existing == '+' || existing == '.');
                if (!isStarGlyph)
                    canvas[cy][lx + i] = lbl[i];
            }
        }
    }

    {
        float q0 = m_minA;
        float q1 = m_minA + (m_maxA - m_minA) / 3.0f;
        float q2 = m_minA + (m_maxA - m_minA) * 2.0f / 3.0f;
        float q3 = m_maxA;

        char rbuf[256];
        std::snprintf(rbuf, sizeof(rbuf),
            "  |%-8.0f%*s%-8.0f%*s%-8.0f%*s%8.0f|",
            q0,
            m_cfg.width/3 - 8, "",
            q1,
            m_cfg.width/3 - 8, "",
            q2,
            m_cfg.width/3 - 8, "",
            q3);
        out << rbuf << '\n';
    }

    for (int r = 0; r < m_cfg.height; ++r) {
        float worldB = lerp((float)r, 0.0f, (float)(m_cfg.height - 1),
                            m_maxB, m_minB);
        char side[16];
        std::snprintf(side, sizeof(side), "%+6.0f|", worldB);
        out << side << canvas[r] << "|\n";
    }

    out << "  +" << std::string(m_cfg.width, '-') << "+\n";

    if (m_cfg.showLegend) {
        out << "\n  LEGEND:  * very bright (mag<-0.5)  "
               "o bright (mag<1.0)  + medium (mag<3.0)  . dim\n";
        out << "  STARS PLOTTED: " << m_stars.size() << '\n';

        out << "\n  ID  Name               Mag     Temp(K)  Glyph\n";
        out << "  " << std::string(50, '-') << '\n';
        for (const auto& s : m_stars) {
            char row[128];
            std::snprintf(row, sizeof(row),
                "  %-3d %-18s %+7.2f  %8.0f    %c\n",
                (int)s.id,
                s.name.c_str(),
                (double)s.magnitude,
                (double)s.temperature,
                glyphForMag(s.magnitude));
            out << row;
        }
    }

    out << border << "\n\n";
    return out.str();
}

void AsciiStarMap::print() const {
    std::cout << render();
}

}