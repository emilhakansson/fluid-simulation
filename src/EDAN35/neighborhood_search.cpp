	 #include <vector>
#include <glm/glm.hpp>
#include <map>
#include <algorithm>
#include <functional>
#include <iostream>

#pragma omp parallel for

namespace edan35 {
	class NeighborhoodSearch {

		struct Entry {

			public:
				Entry() : index(0), cell_key(0) {};
				Entry(int index, int cell_key) {
					this->index = index;
					this->cell_key = cell_key;
				};
				int index;
				int cell_key;
		};

	public:
		NeighborhoodSearch(std::vector<glm::vec3> positions, float radius) {
			this->positions = positions;
			this->radius = radius;
			this->sqr_radius = pow(radius, 2);
			spatial_lookup = std::vector<Entry>(positions.size());
			start_indices = std::vector<int>(positions.size());
		}

		std::vector<int> getNeighborIndices(glm::vec3 position) {
			std::vector<int> neighbor_indices;
			glm::vec3 cell_coords = positionToCellCoords(position);
			int key = getCellKey(cellHash(cell_coords));
			neighbor_indices.push_back(spatial_lookup[start_indices[key]].index); //  fix this
			for (int i = 0; i < grid_neighbor_offsets.size(); i++) {
				glm::vec3 offset = grid_neighbor_offsets[i];
				glm::vec3 neighbor_cell_coords = cell_coords + offset;
				int neighbor_hash = cellHash(neighbor_cell_coords);
				int neighbor_key = getCellKey(neighbor_hash);
				int start_index = start_indices[neighbor_key];
				for (int i = start_index; i < spatial_lookup.size(); i++) {
					if (spatial_lookup[i].cell_key != neighbor_key) break;

					int particle_index = spatial_lookup[i].index;
					float sqr_dist = pow(length((positions[particle_index] - position)), 2);
					if (sqr_dist <= sqr_radius) {
						neighbor_indices.push_back(particle_index);
					}
				}
			}
			return neighbor_indices;
		}

		void update(std::vector<glm::vec3> positions, float radius) {
			this->positions = positions;
			this->radius = radius;
			this->sqr_radius = radius * radius;

			for (int i = 0; i < positions.size(); i++) {
				glm::vec3 cell_coords = positionToCellCoords(positions[i]);
				int cell_hash = cellHash(cell_coords);
				unsigned int cell_key = getCellKey(cell_hash);
				spatial_lookup[i] = Entry(i, cell_key);
				start_indices[i] = INT_MAX;
			}

			// sort the spatial lookup vector in order of ascending cell keys
			sort(spatial_lookup.begin(), spatial_lookup.end(), [](Entry a, Entry b) { return a.cell_key < b.cell_key; });

			for (int i = 0; i < positions.size(); i++) {
				unsigned int key = spatial_lookup[i].cell_key;
				unsigned int prev_key = i == 0 ? INT_MAX : spatial_lookup[i - 1].cell_key;
				if (key != prev_key) {
					start_indices[key] = i;
				}
			}
		}

		// Given that every grid cell is exactly as large as the particle influence radius,
		// we need to look at the 27 grid cells surrounding our point to cover every possible
		// neighboring particle.
		std::vector<glm::vec3> grid_neighbor_offsets = {
			// "top" layer (y = 1)
			glm::vec3(-1, 1, -1), glm::vec3(-1, 1, 0), glm::vec3(-1, 1, 1),
			glm::vec3(0, 1, -1), glm::vec3(0, 1, 0), glm::vec3(0, 1, 1),
			glm::vec3(1, 1, -1), glm::vec3(1, 1, 0), glm::vec3(1, 1, 1),
			// "middle" layer (y = 0)
			glm::vec3(-1, 0, -1), glm::vec3(-1, 0, 0), glm::vec3(-1, 0, 1),
			glm::vec3(0, 0, -1), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1),
			glm::vec3(1, 0, -1), glm::vec3(1, 0, 0), glm::vec3(1, 0, 1),
			// "bottom" layer (y = -1)
			glm::vec3(-1, -1, -1), glm::vec3(-1, -1, 0), glm::vec3(-1, -1, 1),
			glm::vec3(0, -1, -1), glm::vec3(0, -1, 0), glm::vec3(0, -1, 1),
			glm::vec3(1, -1, -1), glm::vec3(1, -1, 0), glm::vec3(1, -1, 1),
		};
	private:
		std::vector<glm::vec3> positions;
		float radius;
		float sqr_radius;
		std::vector<Entry> spatial_lookup;
		std::vector<int> start_indices;

		glm::vec3 positionToCellCoords(glm::vec3 position) {
			int cell_x = (int)(position.x / radius);
			int cell_y = (int)(position.y / radius);
			int cell_z = (int)(position.z / radius);
			return glm::vec3(cell_x, cell_y, cell_z);
		}

		int getCellKey(int hash) {
			return hash % spatial_lookup.size();
		}

		unsigned int cellHash(glm::vec3 grid_coords) {
			unsigned int a = static_cast<unsigned int>(grid_coords.x) * 15823;
			unsigned int b = static_cast<unsigned int>(grid_coords.y) * 9737333;
			unsigned int c = static_cast<unsigned int>(grid_coords.z) * 769;
			return a + b + c;
		}

	};
}
