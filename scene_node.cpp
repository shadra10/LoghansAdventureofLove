#include <stdexcept>
#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <time.h>
#include <algorithm>

#include "scene_node.h"

namespace game {

SceneNode::SceneNode(const std::string name, const Resource *geometry, const Resource *material, const Resource *texture){

    // Set name of scene node
    name_ = name;
	radius = 0;

	speed = 0;

	draw = true;
	safe = true;


	forward_ = glm::vec3(0.0, 0.0, 0.0) - glm::vec3(0.5, 0.5, 10.0);
	forward_ = -glm::normalize(forward_);
	side_ = glm::cross(glm::vec3(0.0, 1.0, 0.0), forward_);
	side_ = glm::normalize(side_);

    // Set geometry
    if (geometry->GetType() == PointSet){
        mode_ = GL_POINTS;
    } else if (geometry->GetType() == Mesh){
        mode_ = GL_TRIANGLES;
    } else {
        throw(std::invalid_argument(std::string("Invalid type of geometry")));
    }

    array_buffer_ = geometry->GetArrayBuffer();
    element_array_buffer_ = geometry->GetElementArrayBuffer();
    size_ = geometry->GetSize();

    // Set material (shader program)
    if (material->GetType() != Material){
        throw(std::invalid_argument(std::string("Invalid type of material")));
    }

    material_ = material->GetResource();

    // Set texture
    if (texture){
        texture_ = texture->GetResource();
    } else {
        texture_ = 0;
    }

    // Other attributes
    scale_ = glm::vec3(1.0, 1.0, 1.0);

	parent = NULL;
}


SceneNode::~SceneNode(){
}


const std::string SceneNode::GetName(void) const {

    return name_;
}


void SceneNode::takeDamage(int) {

}


bool SceneNode::isSafe() {
	if (position_.x > -20 && position_.x < 20 && position_.z > -20 && position_.z < 20)
	safe = true;
	else safe = false;

	return safe;
}

float SceneNode::GetRadius(void){

	return radius;
}

glm::vec3 SceneNode::GetPosition(void) const {

    return position_;
}


glm::quat SceneNode::GetOrientation(void) const {

    return orientation_;
}


glm::vec3 SceneNode::GetScale(void) const {

    return scale_;
}


void SceneNode::SetPosition(glm::vec3 position){

    position_ = position;

	
}


void SceneNode::SetOrientation(glm::quat orientation){

    orientation_ = orientation;
}


void SceneNode::SetScale(glm::vec3 scale){

    scale_ = scale;
}


void SceneNode::Translate(glm::vec3 trans){

    position_ += trans;
}


void SceneNode::Rotate(glm::quat rot){

    orientation_ *= rot;
}


void SceneNode::Scale(glm::vec3 scale){

    scale_ *= scale;
}


GLenum SceneNode::GetMode(void) const {

    return mode_;
}


GLuint SceneNode::GetArrayBuffer(void) const {

    return array_buffer_;
}


GLuint SceneNode::GetElementArrayBuffer(void) const {

    return element_array_buffer_;
}


GLsizei SceneNode::GetSize(void) const {

    return size_;
}


GLuint SceneNode::GetMaterial(void) const {

    return material_;
}

glm::vec3 SceneNode::GetForward(void) const {

	glm::vec3 current_forward = orientation_ * forward_;
	return -current_forward; // Return -forward since the camera coordinate system points in the opposite direction
}

void SceneNode::SetForward(glm::vec3 forward) {

	forward_ = forward;
}

glm::vec3 SceneNode::GetSide(void) const {

	glm::vec3 current_side = orientation_ * side_;
	return current_side;
}


glm::mat4 SceneNode::Draw(Camera *camera, glm::mat4 parent_transf){

	if (draw) {
		if ((array_buffer_ > 0) && (material_ > 0)) {
			// Select proper material (shader program)
			glUseProgram(material_);

			// Set geometry to draw
			glBindBuffer(GL_ARRAY_BUFFER, array_buffer_);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer_);

			// Set globals for camera
			camera->SetupShader(material_);

			// Set world matrix and other shader input variables
			glm::mat4 transf = SetupShader(material_, parent_transf);

			// Draw geometry
			if (mode_ == GL_POINTS) {
				glDrawArrays(mode_, 0, size_);
			}
			else {
				glDrawElements(mode_, size_, GL_UNSIGNED_INT, 0);
			}

			return transf;
		}
		else {
			glm::mat4 rotation = glm::mat4_cast(orientation_);
			glm::mat4 translation = glm::translate(glm::mat4(1.0), position_);
			glm::mat4 transf = parent_transf * translation * rotation;
			return transf;
		}
	}
}





void SceneNode::Update(void){


	
    // Do nothing for this generic type of scene node
}


glm::mat4 SceneNode::SetupShader(GLuint program, glm::mat4 parent_transf){

    // Set attributes for shaders
    GLint vertex_att = glGetAttribLocation(program, "vertex");
    glVertexAttribPointer(vertex_att, 3, GL_FLOAT, GL_FALSE, 11*sizeof(GLfloat), 0);
    glEnableVertexAttribArray(vertex_att);

    GLint normal_att = glGetAttribLocation(program, "normal");
    glVertexAttribPointer(normal_att, 3, GL_FLOAT, GL_FALSE, 11*sizeof(GLfloat), (void *) (3*sizeof(GLfloat)));
    glEnableVertexAttribArray(normal_att);

    GLint color_att = glGetAttribLocation(program, "color");
    glVertexAttribPointer(color_att, 3, GL_FLOAT, GL_FALSE, 11*sizeof(GLfloat), (void *) (6*sizeof(GLfloat)));
    glEnableVertexAttribArray(color_att);

    GLint tex_att = glGetAttribLocation(program, "uv");
    glVertexAttribPointer(tex_att, 2, GL_FLOAT, GL_FALSE, 11*sizeof(GLfloat), (void *) (9*sizeof(GLfloat)));
    glEnableVertexAttribArray(tex_att);

    // World transformation
    glm::mat4 scaling = glm::scale(glm::mat4(1.0), scale_);
    glm::mat4 rotation = glm::mat4_cast(orientation_);
    glm::mat4 translation = glm::translate(glm::mat4(1.0), position_);
	glm::mat4 transf = parent_transf * translation * rotation;// * scaling;
	glm::mat4 local_transf = transf * scaling;

    GLint world_mat = glGetUniformLocation(program, "world_mat");
    glUniformMatrix4fv(world_mat, 1, GL_FALSE, glm::value_ptr(local_transf));

    // Normal matrix
    glm::mat4 normal_matrix = glm::transpose(glm::inverse(transf));
    GLint normal_mat = glGetUniformLocation(program, "normal_mat");
    glUniformMatrix4fv(normal_mat, 1, GL_FALSE, glm::value_ptr(normal_matrix));

    // Texture
    if (texture_){
        GLint tex = glGetUniformLocation(program, "texture_map");
        glUniform1i(tex, 0); // Assign the first texture to the map
        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, texture_); // First texture we bind
        // Define texture interpolation
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


		//if (this->name_ == "Ground") {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//}


		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    // Timer
    GLint timer_var = glGetUniformLocation(program, "timer");
    double current_time = glfwGetTime();
    glUniform1f(timer_var, (float) current_time);

	return transf;
}



void SceneNode::SetMaterial(const Resource *material) {

	this->material_ = material->GetResource();


}

void SceneNode::SetTexture(const Resource *texture) {

	this->texture_ = texture->GetResource();


}

void SceneNode::AddChild(SceneNode *node) {
	children.push_back(node);
	node->parent = this;
}

std::vector<SceneNode *>::const_iterator SceneNode::children_begin() const {

	return children.begin();
}


std::vector<SceneNode *>::const_iterator SceneNode::children_end() const {

	return children.end();
}

void SceneNode::removeChild(SceneNode * child) {
	std::vector<SceneNode*>::iterator position = std::find(children.begin(), children.end(), child);
	if (position != children.end()) {
		children.erase(position);
	}

}

} // namespace game;
