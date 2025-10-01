#pragma once
#include <cstdint>
#include <map>
#include <vector>
#include <iostream>

template<typename GameStateType, typename GameEventType, typename SerializedGameStateType>
class GameBase {
protected:
    // usage: m_event_history[tick][event_index].first() = player id, not all events use this
    // usage: m_event_history[tick][event_index].first() = event
    std::map<uint32_t, std::vector<std::pair<uint32_t, GameEventType>>> m_event_history;
    std::map<uint32_t, GameStateType> m_state_history;

public:
    void AddEvent(GameEventType event, uint32_t id, uint32_t tick) {
        m_event_history[tick].push_back({id, event});
    }

    GameStateType ApplyEventsAsOneTick(const GameStateType& start_state) {
        GameStateType result_state = start_state;
        for (auto& [tick, events] : m_event_history) {
            for (auto& [id, event] : events) {
                ApplyEvent(result_state, event, id);
            }
        }
        UpdateGameLogic(result_state);
        m_event_history.clear();
        
        return result_state;
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
            UpdateGameLogic(result_state);
            currentTick++;
        }

        return result_state;
    }

    void DropEventHistory(uint32_t last_dropped_tick) {
        for (auto& [tick, events] : m_event_history) {
            if (tick <= last_dropped_tick) {
                m_event_history.erase(tick);
            }
        }
    }

    virtual void ApplyEvent(GameStateType& state, const GameEventType& event, uint32_t id) = 0;
    virtual void Draw(const GameStateType& state, const void* data) = 0;
    virtual void UpdateGameLogic(GameStateType& state) = 0;

    virtual SerializedGameStateType Serialize(const GameStateType& state) = 0;
    virtual GameStateType Deserialize(SerializedGameStateType data) = 0;

    virtual GameStateType Lerp(const GameStateType& state1, const GameStateType& state2, float alpha, const void* data) = 0;
    //virtual GameStateType ConditionalLerp(const GameStateType& state_0, const GameStateType& state1, const GameStateType& state2, float alpha, const void* data) = 0;
};
