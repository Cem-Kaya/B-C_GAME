#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP


class drawable {
public:
	virtual void draw()  = 0;
	virtual glm::mat4  getTransform() = 0;

	virtual void set_light_pos(glm::vec3 new_light_pos) = 0;
	virtual void set_view_pos(glm::vec3 new_view_pos)   = 0;
	virtual void set_view(glm::mat4 cam_mat) = 0;
	virtual void set_perspective(glm::mat4 perspective) = 0;
	


	virtual void translate(float x, float y, float z) = 0;
	virtual void scale(float x, float y, float z) = 0;
	virtual void rotate(float x, float y, float z) = 0; // degrees

	virtual glm::vec3 position() = 0;
	virtual glm::vec3 scale() = 0;
	virtual glm::vec3 rotation() = 0;


	virtual void physics_update(float delta_time) = 0;
	virtual int  intersect(drawable* other) = 0;
	virtual float get_max_x() = 0;
	virtual float get_min_x() = 0;
	virtual float get_max_y() = 0;
	virtual float get_min_y() = 0;
	virtual float get_max_z() = 0;
	virtual float get_min_z() = 0;
	virtual glm::vec3 get_origin() = 0;

	virtual void set_movable(bool in) = 0 ;
	virtual bool get_movable() = 0;
	virtual glm::vec3 get_current_momentum() = 0;
	virtual void set_current_momentum(glm::vec3 momentum_awakens) = 0;
	virtual void set_terminal_momentum(glm::vec3 terminal_awakens) = 0;
	virtual glm::vec3 get_force() = 0;
	virtual void set_force(glm::vec3 force_awakens) = 0;
	virtual glm::vec3 get_min() = 0;
	virtual glm::vec3 get_max() = 0;


	virtual string get_name() = 0;
	virtual void set_name(string new_name) = 0;
	virtual void got_hit() = 0;
	virtual void not_hit() = 0;
	virtual bool get_is_hit() = 0;

};


#endif
