#include "DNAFlatBuilding.h"
#include <nodePathCollection.h>
#include <decalEffect.h>

DNAFlatBuilding::DNAFlatBuilding(string name) : DNANode(name), m_has_door(false),
												m_width(0) {}
DNAFlatBuilding::~DNAFlatBuilding(void) {}

int DNAFlatBuilding::current_wall_height = 0;

void DNAFlatBuilding::make_from_dgi(DatagramIterator &dgi, DNAStorage *store)
{
	DNANode::make_from_dgi(dgi, store);
	m_width = dgi.get_uint8();
	m_has_door = dgi.get_bool();
};

void DNAFlatBuilding::traverse(NodePath np, DNAStorage* store)
{
	DNAFlatBuilding::current_wall_height = 0;
	NodePath node = np.attach_new_node(m_name);
	stringstream ss;
	ss << m_name << "-internal";
	NodePath internal_node = node.attach_new_node(ss.str());
	m_scale->set_x(m_width);
	internal_node.set_scale(*m_scale);
	node.set_pos(*m_pos);
	node.set_hpr(*m_hpr);

	for (comp_vec_t::iterator child = m_children.begin(); child != m_children.end(); child++)
	{
        if ((*child)->is_wall())
		{
			(*child)->traverse(internal_node, store);
		}

		else
		{
			(*child)->traverse(node, store);
		}
	};

	if (DNAFlatBuilding::current_wall_height != 0)
	{
		NodePath* result = store->find_node("wall_camera_barrier");
		// to do: check
		NodePath camera_barrier = result->copy_to(internal_node);
		camera_barrier.set_scale(LVector3f(1, 1, DNAFlatBuilding::current_wall_height));
		// to do: setup suit and cogdo
		internal_node.flatten_strong();
		// to do: knock knock door
		camera_barrier.wrt_reparent_to(np);

		NodePathCollection wall_collection = internal_node.find_all_matches("wall*");
		NodePath wall_holder = node.attach_new_node("wall_holder");
		NodePath wall_decal = node.attach_new_node("wall_decal");
		NodePathCollection window_collection = internal_node.find_all_matches("**/cornice*_d");
		NodePathCollection door_collection = internal_node.find_all_matches("**/door*");
        NodePathCollection cornice_collection = internal_node.find_all_matches("**/cornice*_d");

		wall_collection.reparent_to(wall_holder);
		window_collection.reparent_to(wall_decal);
		door_collection.reparent_to(wall_decal);
		cornice_collection.reparent_to(wall_decal);

		short num_children = wall_holder.get_num_children();
		for (short i = 0; i < num_children; i++)
		{
			NodePath x = wall_holder.get_child(i);
			x.clear_tag("DNACode");
			x.clear_tag("DNARoot");
		};
		
		wall_holder.flatten_strong();
		wall_decal.flatten_strong();

		NodePath holder_child0 = wall_holder.get_child(0);
		wall_decal.get_children().reparent_to(holder_child0);
		holder_child0.reparent_to(internal_node);
		holder_child0.set_effect(DecalEffect::make());

		wall_holder.remove_node();
		wall_decal.remove_node();
	};
};