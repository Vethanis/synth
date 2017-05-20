#include "imguimesh.h"
#include "myglheaders.h"

/*
// for reference
struct ImDrawVert
{
    ImVec2  pos;
    ImVec2  uv;
    ImU32   col;
};

typedef unsigned short ImDrawIdx;
*/

namespace imguimesh{

    void draw(ImVector<ImDrawVert>& verts, ImVector<ImDrawIdx>& indices){
        unsigned vao, vbo, ibo;
        
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ibo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (void*)sizeof(ImVec2));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (void*)(2 * sizeof(ImVec2)) );

        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(ImDrawVert), &verts.front(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(ImDrawIdx), &indices.front(), GL_STATIC_DRAW);

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);

        glDeleteBuffers(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

};