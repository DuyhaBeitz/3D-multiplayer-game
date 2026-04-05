#include "SpacePartition.hpp"

void PartitionGrid::add(PartitionUnit *unit) {
    // Determine which grid cell it's in.
    int cellX = CoordIntoCellCapped(unit->x);
    int cellY = CoordIntoCellCapped(unit->y);

    unit->curr_cell_x = cellX;
    unit->curr_cell_y = cellY;

    // Add to the front of list for the cell it's in.
    unit->prev = nullptr;
    unit->next = m_cells[cellX][cellY];
    m_cells[cellX][cellY] = unit;

    if (unit->next) {
        unit->next->prev = unit;
    }
}

void PartitionGrid::remove(PartitionUnit *unit) {
    // Unlink it from the list of its old cell.
    if (unit->prev) unit->prev->next = unit->next;
    if (unit->next) unit->next->prev = unit->prev;

    int cellX = unit->curr_cell_x;
    int cellY = unit->curr_cell_y;
    // If it's the head of a list, remove it.
    if (m_cells[cellX][cellY] == unit) {
        m_cells[cellX][cellY] = unit->next;
    }
}

void PartitionGrid::unit_with_grid(PartitionUnit *unit, float x, float y, void* user_data) const {
    int cellX = CoordIntoCellCapped(x);
    int cellY = CoordIntoCellCapped(y);
    PartitionUnit* other = m_cells[cellX][cellY];
    handle_partition_unit(unit, other, user_data);

    // Also try the neighboring cells.
    int max = NUM_CELLS -1;
    if (cellX > 0 && cellY > 0) handle_partition_unit(unit, m_cells[cellX - 1][cellY - 1], user_data);
    if (cellX < max && cellY < max) handle_partition_unit(unit, m_cells[cellX + 1][cellY + 1], user_data);

    if (cellX > 0) handle_partition_unit(unit, m_cells[cellX - 1][cellY], user_data);
    if (cellX < max) handle_partition_unit(unit, m_cells[cellX + 1][cellY], user_data);

    if (cellY > 0) handle_partition_unit(unit, m_cells[cellX][cellY - 1], user_data);
    if (cellY < max) handle_partition_unit(unit, m_cells[cellX][cellY + 1], user_data);

    if (cellX > 0 && cellY < max) handle_partition_unit(unit, m_cells[cellX - 1][cellY + 1], user_data);
    if (cellX < max && cellY > 0) handle_partition_unit(unit, m_cells[cellX + 1][cellY - 1], user_data);
}

void PartitionGrid::iterate_cells(void* user_data) const {
    for (int x = 0; x < NUM_CELLS; x++) {
        for (int y = 0; y < NUM_CELLS; y++) {
            handle_cell(x, y, user_data);
        }
    }
}

void PartitionGrid::handle_cell(int x, int y, void* user_data) const {
    PartitionUnit* unit = m_cells[x][y];
    while (unit) {
        // Handle other units in this cell.
        handle_partition_unit(unit, unit->next, user_data);

        // Also try the neighboring cells.
        /*
        We only look at half of the neighbors
        for the same reason that the inner loop starts after the current unit 
        — to avoid comparing each pair of units twice.
        */
        if (x > 0 && y > 0) handle_partition_unit(unit, m_cells[x - 1][y - 1], user_data);
        if (x > 0) handle_partition_unit(unit, m_cells[x - 1][y], user_data);
        if (y > 0) handle_partition_unit(unit, m_cells[x][y - 1], user_data);
        if (x > 0 && y < NUM_CELLS - 1)
        {
        handle_partition_unit(unit, m_cells[x - 1][y + 1], user_data);
        }

        unit = unit->next;
    }
}

void PartitionGrid::handle_partition_unit(PartitionUnit *unit, PartitionUnit *other, void* user_data) const {
    while (other) {
        if (m_handle_pair) m_handle_pair(unit, other, user_data);
        other = other->next;
    }
}

void PartitionGrid::move(PartitionUnit *unit, double x, double y) {
    // See which cell it was in.
    int oldCellX = unit->curr_cell_x;
    int oldCellY = unit->curr_cell_y;

    // See which cell it's moving to.
    int cellX = CoordIntoCellCapped(x);
    int cellY = CoordIntoCellCapped(y);

    unit->x = x;
    unit->y = y;
    
    // If it didn't change cells, we're done.
    if (oldCellX == cellX && oldCellY == cellY) return;

    remove(unit);
    add(unit);
}