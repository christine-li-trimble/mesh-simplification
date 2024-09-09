#pragma once
#include <limits>
#include <vertex.h>
// ========================================
// global constants QEM
// ========================================
using namespace std;

static constexpr float INF_COST = numeric_limits<float>::max();
static constexpr long GHOST_HALF_EDGE = numeric_limits<long>::max() - 5;
static constexpr long INVALID_HALF_EDGE = numeric_limits<long>::max() - 1;
static constexpr long INVALID_VERTEX_INDEX = numeric_limits<long>::max() - 2;
static constexpr long INVALID_EDGE = numeric_limits<long>::max() - 4;
static const vertex GHOST_VERTEX_LOCATION = { 0.0,0.0,0.0 }; 
