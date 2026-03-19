#pragma once

#include <functional>

/*
https://gameprogrammingpatterns.com/spatial-partition.html
*/

struct PartitionUnit;

class PartitionGrid {
public:
    PartitionGrid() {
        // Clear the grid.
        for (int x = 0; x < NUM_CELLS; x++) {
            for (int y = 0; y < NUM_CELLS; y++) {
                m_cells[x][y] = nullptr;
            }
        }
    }

    void add(PartitionUnit* unit);
    //void iterate_cells() const;

    void unit_with_grid(PartitionUnit* unit, float x, float y) const;

    //void handle_cell(int x, int y) const;

    void handle_partition_unit(PartitionUnit* unit, PartitionUnit* other) const;

    //void move(PartitionUnit* unit, double x, double y);


    static const int NUM_CELLS = 10;
    static const int CELL_SIZE = 100;

    void SetHandlePairFunc(std::function<void(PartitionUnit*, PartitionUnit*)> handle_pair) { m_handle_pair = handle_pair; }

    int CoordIntoCell(float coord) const {
        int cell = (int)((coord+NUM_CELLS/2*PartitionGrid::CELL_SIZE) / PartitionGrid::CELL_SIZE);
        int min = 0;
        int max = NUM_CELLS - 1;
        if (cell < min) cell = min;
        if (cell > max) cell = max;
        return cell;
    }

    int CoordIntoCellCapped(float coord) const {
        int cell = CoordIntoCell(coord);
        cell %= NUM_CELLS;
        return cell;
    }

    float CellIntoCoord(int cell) const { // returns the min corner
        return CELL_SIZE * (cell - NUM_CELLS/2);
    }

private:
    PartitionUnit* m_cells[NUM_CELLS][NUM_CELLS];
    std::function<void(PartitionUnit*, PartitionUnit*)> m_handle_pair = nullptr;
};

struct PartitionUnit {
public:
    PartitionUnit(PartitionGrid* grid_, double x_, double y_)
    : grid(grid_), x(x_), y(y_), prev(nullptr), next(nullptr)
    {
    }


    // void move(double x, double y) {
    //     grid->move(this, x, y);
    // }

    void* user_data;

    double x = 0;
    double y = 0;

    PartitionGrid* grid = nullptr;
    PartitionUnit* prev = nullptr;
    PartitionUnit* next = nullptr;
};