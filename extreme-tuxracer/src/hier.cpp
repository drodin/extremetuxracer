/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
 * 
 * Copyright (C) 1999-2001 Jasmin F. Patry
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "hier.h"
#include "hier_util.h"
#include "game_config.h"

#include "ppgltk/alg/defs.h"

/*
 * These hash tables map from names to node pointers and material data, resp.
 */
Tcl_HashTable g_hier_node_table;
Tcl_HashTable g_hier_material_table;


/*
 * State variables
 */


/* Default Material */
material_t g_hier_default_material ={ 
				pp::Color(0., 0., 1.),  /* diffuse color  = blue */
				pp::Color(0., 0., 0.),  /* specular color = black */
				0.0              /* specular exp. = 0.0 */
			};
/*
 * Functions used to access and update the name-to-pointer hash tables
 */

/* Add a new node name to the node name hash table.
   node_name contains the child's name only. */
static int 
add_scene_node( const char *parent_name, const char *node_name, scene_node_t *node ) 
{
    Tcl_HashEntry *entry;
    int newEntry;
    char new_name[1024];

    /* Add the current node to the hash table 
    */
    if (0 == strcmp(parent_name,":")) {
        sprintf(new_name, ":%s", node_name);
    } else {
        sprintf(new_name, "%s:%s", parent_name, node_name);
    }

    node->name = (char*)malloc( strlen(new_name) + 1 );
    node->name = strcpy(node->name, new_name);

    entry = Tcl_CreateHashEntry(&g_hier_node_table,new_name,&newEntry);

    if (newEntry) { 
        Tcl_SetHashValue(entry, node);
    } else {
        return TCL_ERROR;
    }

    return TCL_OK;
}

/* Get node pointer from node name */
int 
get_scene_node( const char *node_name, scene_node_t **node ) 
{
    Tcl_HashEntry *entry;
  
    entry = Tcl_FindHashEntry(&g_hier_node_table, node_name);
    if (0 == entry) {
        if(0 == strcmp(node_name, ":")) {
            /* the root has been specified 
             */
            *node = 0;
        } else {
            return TCL_ERROR;
        }
    } else {
        *node = (scene_node_t*)Tcl_GetHashValue(entry);
    }

    return TCL_OK;
}

/* Add a new material name to the material name hash table. */
int 
add_material( const char *mat_name, material_t *mat ) 
{
    Tcl_HashEntry *entry;
    int newEntry;

    entry = Tcl_CreateHashEntry(&g_hier_material_table,mat_name,&newEntry);
    if (newEntry) {
        Tcl_SetHashValue(entry, mat);
    } else {
        return TCL_ERROR;
    }

    return TCL_OK;
}

/* Get material pointer from material name */
int 
get_material( const char *mat_name, material_t **mat ) 
{
    Tcl_HashEntry *entry;

    entry = Tcl_FindHashEntry(&g_hier_material_table, mat_name);
    if (0 == entry) {
        return TCL_ERROR;
    } else {
        *mat = (material_t*)Tcl_GetHashValue(entry);
    }

    return TCL_OK;
}

/* Creates a new node, add the node to the hash table, and inserts the
   node into the DAG.  Default values are given to all fields except
   the type-specific ones (geom, param).  */
char* create_scene_node( const char *parent_name, const char *child_name, 
			 scene_node_t **node )
{
    scene_node_t *parent, *child;
    if ( get_scene_node( parent_name, &parent ) != TCL_OK ) {
        return "Parent node does not exist";
    } 

    /* Create node */
    child = (scene_node_t *)malloc( sizeof( scene_node_t ) );

    /* Initialize node */
    child->parent = parent;
    child->next = NULL;
    child->child = NULL;
    child->mat = NULL;
    child->render_shadow = true;
    child->eye = false;
    child->trans.makeIdentity();
    child->invtrans.makeIdentity();

    if ( add_scene_node( parent_name, child_name, child ) != TCL_OK ) {
        free( child );
        return "Child already exists";
    } 


    /* Add node to parent's children */
    if ( parent != NULL ) {
        if ( parent->child == NULL ) {
            parent->child = child;
        } else {
            for (parent = parent->child; parent->next != NULL; 
                 parent = parent->next) {/* do nothing */}
            parent->next = child;
        } 
    } 

    *node = child;
    return NULL;
} 

char*
reset_scene_node( char *node ) 
{  
    scene_node_t *nodePtr;

    if ( get_scene_node( node, &nodePtr ) != TCL_OK ) {
        return "No such node";
    } 

    nodePtr->trans.makeIdentity();
    nodePtr->invtrans.makeIdentity();

    return NULL;
}

char*
rotate_scene_node( const char *node, char axis, double angle ) 
{
    scene_node_t *nodePtr;
    pp::Matrix rotMatrix;

    if ( get_scene_node( node, &nodePtr ) != TCL_OK ) {
        return "No such node";
    } 

    rotMatrix.makeRotation( angle, axis );
    nodePtr->trans=nodePtr->trans*rotMatrix;
	
    rotMatrix.makeRotation( -angle, axis );
    nodePtr->invtrans=rotMatrix*nodePtr->invtrans;

    return NULL;
}

char*
translate_scene_node( const char *node, pp::Vec3d vec ) 
{
    scene_node_t *nodePtr;
    pp::Matrix xlateMatrix;

    if ( get_scene_node( node, &nodePtr ) != TCL_OK ) {
        return "No such node";
    } 

    xlateMatrix.makeTranslation( vec.x, vec.y, vec.z );
	nodePtr->trans=nodePtr->trans*xlateMatrix;
	
    xlateMatrix.makeTranslation( -vec.x, -vec.y, -vec.z );
    nodePtr->invtrans=xlateMatrix*nodePtr->invtrans;

    return NULL;
}

char*
scale_scene_node( const char *node, pp::Vec3d center, double factor[3] ) 
{
    scene_node_t *nodePtr;
    pp::Matrix matrix;

    if ( get_scene_node( node, &nodePtr ) != TCL_OK ) {
        return "No such node";
    } 

    matrix.makeTranslation( -center.x, -center.y, -center.z );
    nodePtr->trans=nodePtr->trans*matrix;
	
    matrix.makeTranslation( center.x, center.y, center.z );
    nodePtr->invtrans=matrix*nodePtr->invtrans;

    matrix.makeScaling( factor[0], factor[1], factor[2] );
    nodePtr->trans=nodePtr->trans*matrix;
	
    matrix.makeScaling( 1./factor[0], 1./factor[1], 1./factor[2] );
    nodePtr->invtrans=matrix*nodePtr->invtrans;

    matrix.makeTranslation( center.x, center.y, center.z );
    nodePtr->trans=nodePtr->trans*matrix;
	
    matrix.makeTranslation( -center.x, -center.y, -center.z );
    nodePtr->invtrans=matrix*nodePtr->invtrans;

    return NULL;
}

char*
transform_scene_node( char *node, pp::Matrix mat, pp::Matrix invmat ) 
{
    scene_node_t *nodePtr;

    if ( get_scene_node( node, &nodePtr ) != TCL_OK ) {
        return "No such node";
    } 

    nodePtr->trans=nodePtr->trans*mat;
    nodePtr->invtrans=invmat*nodePtr->invtrans;

    return NULL;
}

char*
set_scene_node_material( const char *node, const char *mat ) 
{
    material_t *matPtr;
    scene_node_t *nodePtr;

    if ( get_scene_node( node, &nodePtr ) != TCL_OK ) {
        return "No such node";
    } 

    if ( get_material( mat, &matPtr ) != TCL_OK ) {
        return "No such material";
    } 

    nodePtr->mat = matPtr;

    return NULL;
}

char*
set_scene_node_shadow_state( const char *node, const char *state ) 
{
    scene_node_t *nodePtr;

    if ( get_scene_node( node, &nodePtr ) != TCL_OK ) {
        return "No such node";
    } 

    if ( strcmp( state, "off" ) == 0 ) {
	nodePtr->render_shadow = false;
    } else if ( strcmp ( state, "on" ) == 0 ) {
	nodePtr->render_shadow = true;
    } else {
	return "Shadow state must be 'on' or 'off'";
    }

    return NULL;
}

char*
set_scene_node_eye( const char *node, const char *which_eye )
{
    scene_node_t *nodePtr;

    if ( get_scene_node( node, &nodePtr ) != TCL_OK ) {
        return "No such node";
    } 

    if ( strcmp( which_eye, "right" ) == 0 ) {
	nodePtr->eye = true;
	nodePtr->which_eye = TuxRightEye;
    } else if ( strcmp ( which_eye, "left" ) == 0 ) {
	nodePtr->eye = true;
	nodePtr->which_eye = TuxLeftEye;
    } else {
	return "'eye' must be right or left";
    }

    return NULL;
}

char*
create_tranform_node( const char *parent_name, const char *child_name ) 
{
    scene_node_t *node;
    char *msg;

    msg = create_scene_node(parent_name, child_name, &node);
    if ( msg != NULL ) {
        return msg;
    } 

    node->geom = Empty;

    return NULL;
}

char*
create_sphere_node( const char *parent_name, const char *child_name, double resolution ) 
{
    scene_node_t *node;
    char *msg;

    msg = create_scene_node(parent_name, child_name, &node);
    if ( msg != NULL ) {
        return msg;
    } 

    node->geom = Sphere;
    node->param.sphere.radius = 1.0;
    node->param.sphere.divisions = MIN( 
	MAX_SPHERE_DIVISIONS, MAX( 
	    MIN_SPHERE_DIVISIONS, 
	    ROUND_TO_NEAREST( getparam_tux_sphere_divisions() * resolution ) 
	    ) );

    return NULL;
}

char*
create_material( const char *mat, pp::Color diffuse, 
		 pp::Color specular, double specular_exp ) 
{
    material_t *matPtr;

    matPtr = (material_t *)malloc( sizeof( material_t ) );

    matPtr->diffuse.r = diffuse.r;
    matPtr->diffuse.g = diffuse.g;
    matPtr->diffuse.b = diffuse.b;
    matPtr->diffuse.a = 1.0;

    matPtr->specular.r = specular.r;
    matPtr->specular.g = specular.g;
    matPtr->specular.b = specular.b;
    matPtr->specular.a = 1.0;

    matPtr->specular_exp = specular_exp;

    if ( add_material( mat, matPtr ) != TCL_OK ) {
        free( matPtr );
        return "Material already exists";
    } 

    return NULL;
}

void initialize_scene_graph() 
{
    /* Initialize state */

    g_hier_default_material.diffuse.r = 0.0;
    g_hier_default_material.diffuse.g = 0.0;
    g_hier_default_material.diffuse.b = 1.0;
    g_hier_default_material.diffuse.a = 1.0;

    g_hier_default_material.specular.r = 0.0;
    g_hier_default_material.specular.g = 0.0;
    g_hier_default_material.specular.b = 0.0;
    g_hier_default_material.specular.a = 1.0;

    g_hier_default_material.specular_exp = 0.0;

    Tcl_InitHashTable(&g_hier_node_table,TCL_STRING_KEYS);
    Tcl_InitHashTable(&g_hier_material_table,TCL_STRING_KEYS);
}

void draw_scene_graph( char *node )
{
    scene_node_t *nodePtr;

    if ( get_scene_node( node, &nodePtr ) != TCL_OK ) {
        handle_error( 1, "draw_scene_graph: No such node `%s'", node );
    } 

    traverse_dag( nodePtr, &g_hier_default_material );
    
} 

bool collide( char *node, pp::Polyhedron ph )
{
    scene_node_t *nodePtr;
    pp::Matrix mat, invmat;

    mat.makeIdentity();
    invmat.makeIdentity();

    if ( get_scene_node( node, &nodePtr ) != TCL_OK ) {
        handle_error( 1, "draw_scene_graph: No such node `%s'", node );
    } 

    return check_polyhedron_collision_with_dag( nodePtr, mat, invmat, ph );
}
