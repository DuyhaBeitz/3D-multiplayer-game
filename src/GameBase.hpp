#pragma once
#include <cstdint>
#include <map>
#include <vector>

template<typename GameStateType, typename GameEventType, typename SerializedGameStateType>
class GameBase {
protected:
    // usage: m_event_history[tick][event_index].first() = player id, not all events use this
    // usage: m_event_history[tick][event_index].second() = event
    std::map<uint32_t, std::vector<std::pair<uint32_t, GameEventType>>> m_event_history{};
    std::map<uint32_t, GameStateType> m_state_history{};
    uint32_t m_tick;

public:
    void AddEvent(GameEventType event, uint32_t id, uint32_t tick) {
        m_event_history[tick].push_back({id, event});
    }

    GameStateType ApplyEvents(const GameStateType& start_state, uint32_t start_tick, uint32_t end_tick) {        
        GameStateType result_state = start_state;
        uint32_t currentTick = start_tick;

        while (currentTick < end_tick) {
            if (m_event_history.find(currentTick) != m_event_history.end()) {
                for (auto& [id, event] : m_event_history[currentTick]) {
                    ApplyEvent(result_state, event, id);
                }
            }
            UpdateGameLogic(result_state, currentTick);
            currentTick++;
        }

        return result_state;
    }

    void DropEventHistory(uint32_t last_dropped_tick) {
        for (auto it = m_event_history.begin(); it != m_event_history.end(); ) {
            if (it->first <= last_dropped_tick) {
                it = m_event_history.erase(it);
            } else {
                ++it;
            }
        }
    }

    virtual void ApplyEvent(GameStateType& state, const GameEventType& event, uint32_t id) = 0;
    virtual void UpdateGameLogic(GameStateType& state, uint32_t tick) = 0;

    virtual SerializedGameStateType Serialize(const GameStateType& state) = 0;
    virtual GameStateType Deserialize(SerializedGameStateType data) = 0;

    virtual GameStateType Lerp(const GameStateType& state1, const GameStateType& state2, float alpha, const void* data) = 0;
};
