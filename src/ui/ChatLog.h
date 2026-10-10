#pragma once

#include <bitset>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace eqgfx::ui {

struct ChatFilter
{
	const char* key;
	const char* label;
	const char* group;
};

constexpr size_t kMaxChatFilters = 192;
using ChatFilterSet = std::bitset<kMaxChatFilters>;

std::span<const ChatFilter> ChatFilters();
int           ChatFilterIndex(std::string_view key);
ChatFilterSet AllChatFilters();
ChatFilterSet ChatFilterGroup(std::string_view group);
ChatFilterSet ChatFiltersForLegacyMask(uint32_t mask);

struct ChatLine
{
	std::string text;
	uint32_t    rgba;
	uint16_t    filter;
};

void AddEqChat(const char* line, uint32_t color);
void AddMqChat(const char* line, int color);

void DrainChat(std::vector<ChatLine>& into, size_t max);

void QueueChatLink(const std::string& line, size_t linkStart, size_t linkLength);
void RunChatLinks();

} // namespace eqgfx::ui
