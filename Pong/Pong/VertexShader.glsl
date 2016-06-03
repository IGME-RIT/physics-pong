/*
Title: Pong Game
File Name: VertexShader.glsl
Copyright © 2015
Original authors: Srinivasan Thiagarajan
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Description:
This example builds on the previous example "Circle-AABB 2D collision detection" to develop
a small pong game.

We use collision aresolution and decoupling algorithm for collisions between the ball and the paddle.
For the boundaries, we use 3 giant sized blocks on the left, right and top. The collision with the
blocks simply reverse the component perpendicular to the face. This is what happens when a object collides with an
immovable object. The same result can be obtained using the conventional formula when the object's
mass is susbtituted with infinity i.e. inverse mass = 0.

The game has a difficulty mechanic which ups the speed after every 4 collisions.
The change in difficulty is reflected by a change in the colors. If the Ball falls
through, the difficulty is reset.

Use "A" and "D" to move the paddle left or right.

References:
2D Game Collision Detection by Thomas Schwarzl,
AABB-2D by Brockton Roth
*/


#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
layout(location = 0) in vec3 in_position;	// Get in a vec3 for position
layout(location = 1) in vec4 in_color;		// Get in a vec4 for color

out vec4 color; // Our vec4 color variable containing r, g, b, a

uniform vec3 colorAndPos; // Our uniform MVP matrix to modify our position values

void main(void)
{
	color = in_color + vec4(0.0f,colorAndPos.z,0.0f,0.0f);	// Pass the color through
	gl_Position = vec4(in_position.xy + colorAndPos.xy,0.0f, 1.0); //w is 1.0, also notice cast to a vec4
}