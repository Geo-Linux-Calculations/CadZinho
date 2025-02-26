#include "gui_use.h"

int gui_mirror_interactive(gui_obj *gui){
	if (gui->modal == MIRROR){
		static list_node *list = NULL;
		list_node *current = NULL;
		dxf_node *new_ent = NULL;
		
		if (gui->step == 0) {
			/* try to go to next step */
			gui->step = 1;
			gui->free_sel = 0;
			gui->phanton = NULL;
		}
		/* verify if elements in selection list */
		if (gui->step == 1 && (!gui->sel_list->next || (gui->ev & EV_ADD))){
			/* if selection list is empty, back to first step */
			gui->step = 0;
			gui->free_sel = 1;
		}
		
		if (gui->step == 0){
			/* in first step, select the elements to proccess*/
			gui->en_distance = 0;
			gui->sel_ent_filter = ~DXF_NONE;
			gui_simple_select(gui);
      /* user cancel operation */
      if (gui->ev & EV_CANCEL){
        gui->element = NULL;
        gui_default_modal(gui);
        gui->step = 0;
      }
		}
		else if (gui->step == 1){
			/* user enters first point of reflection line */
			gui->free_sel = 0;
			if (gui->ev & EV_ENTER){
				/* make phantom list */
				list = list_new(NULL, FRAME_LIFE);
				list_clear(list);
				
				/* sweep selection list */
				current = gui->sel_list->next;
				while (current != NULL){
					if (current->data){/* current entity */
						if (((dxf_node *)current->data)->type == DXF_ENT){
							/* get a temporary copy of current entity */
							new_ent = dxf_ent_copy((dxf_node *)current->data, FRAME_LIFE);
							list_node * new_el = list_new(new_ent, FRAME_LIFE);
							if (new_el){
								list_push(list, new_el);
							}
						}
					}
					current = current->next;
				}
				
				
				gui->draw_tmp = 1;
				/* draw phantom */
				gui->phanton = dxf_list_parse(gui->drawing, list, 0, FRAME_LIFE);
				gui->element = NULL;
				gui->draw_phanton = 1;
				gui->en_distance = 1;
				gui->step_x[gui->step + 1] = gui->step_x[gui->step];
				gui->step_y[gui->step + 1] = gui->step_y[gui->step];
				gui->step = 2;
				gui->step_x[gui->step + 1] = gui->step_x[gui->step];
				gui->step_y[gui->step + 1] = gui->step_y[gui->step];
				gui_next_step(gui);
			}
			else if (gui->ev & EV_CANCEL){
				gui_default_modal(gui);
			}
		}
		else{
			/* user enters second point of reflection line */
			if (gui->ev & EV_ENTER){
				/* completes the operation */
				if (gui->sel_list != NULL){
					new_ent = NULL;
					current = gui->sel_list->next;
					if (current != NULL){
						/* add to undo/redo list */
						do_add_entry(&gui->list_do, _l("MIRROR"));
					}
					/* sweep the selection list */
					while (current != NULL){
						if (current->data){ /* current entity */
							if (((dxf_node *)current->data)->type == DXF_ENT){
								/* get a copy of current entity */
								new_ent = dxf_ent_copy((dxf_node *)current->data, DWG_LIFE);
								/* apply modifications */
								dxf_edit_mirror(new_ent, gui->step_x[gui->step], gui->step_y[gui->step], gui->step_x[gui->step - 1], gui->step_y[gui->step - 1]);
								/* draw the new entity */
								new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , DWG_LIFE);
								
								if(!gui->keep_orig){ /* add the new entity in drawing, replacing the old one  */
									dxf_obj_subst((dxf_node *)current->data, new_ent);
									do_add_item(gui->list_do.current, (dxf_node *)current->data, new_ent);
									
									/* for DIMENSIONS - remake the block "picture" */
									dxf_node *blk, *blk_rec, *blk_old, *blk_rec_old;
									if ( dxf_dim_rewrite (gui->drawing, new_ent, &blk, &blk_rec, &blk_old, &blk_rec_old)){
										do_add_item(gui->list_do.current, blk_old, NULL);
										do_add_item(gui->list_do.current, blk_rec_old, NULL);
										do_add_item(gui->list_do.current, NULL, blk);
										do_add_item(gui->list_do.current, NULL, blk_rec);
									}
								}
								else{ /* maintains the original entity*/
									drawing_ent_append(gui->drawing, new_ent);
									do_add_item(gui->list_do.current, NULL, new_ent);
									
									/* for DIMENSIONS - make the block "picture" */
									dxf_node *blk, *blk_rec;
									if ( dxf_dim_make_blk (gui->drawing, new_ent, &blk, &blk_rec)){
										do_add_item(gui->list_do.current, NULL, blk);
										do_add_item(gui->list_do.current, NULL, blk_rec);
									}
								}
								
								current->data = new_ent; /* replancing in selection list */
							}
						}
						current = current->next;
					}
				}
				gui_first_step(gui);
				gui->step = 1;
			}
			else if (gui->ev & EV_CANCEL){
				gui_first_step(gui);
				gui->step = 1;
			}
			if (gui->ev & EV_MOTION){
				/* make phantom list */
				list = list_new(NULL, FRAME_LIFE);
				list_clear(list);
				
				/* sweep selection list */
				current = gui->sel_list->next;
				while (current != NULL){
					if (current->data){ /* current entity*/
						if (((dxf_node *)current->data)->type == DXF_ENT){
							/* get a temporary copy of current entity */
							new_ent = dxf_ent_copy((dxf_node *)current->data, FRAME_LIFE);
							list_node * new_el = list_new(new_ent, FRAME_LIFE);
							/* apply modifications */
							dxf_edit_mirror(new_ent, gui->step_x[gui->step], gui->step_y[gui->step], gui->step_x[gui->step - 1], gui->step_y[gui->step - 1]);
							if (new_el){
								list_push(list, new_el);
							}
						}
					}
					current = current->next;
				}
				
				gui->phanton = dxf_list_parse(gui->drawing, list, 0, FRAME_LIFE);
			}
		}
	}
	return 1;
}

int gui_mirror_info (gui_obj *gui){
	if (gui->modal == MIRROR) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, _l("Mirror a selection"), NK_TEXT_LEFT);
		nk_checkbox_label(gui->ctx, _l("Keep Original"), &gui->keep_orig);
		
		if (gui->step == 0){
			nk_label(gui->ctx, _l("Select/Add element"), NK_TEXT_LEFT);
		}
		else if (gui->step == 1){
			nk_label(gui->ctx, _l("Set the reflection line"), NK_TEXT_LEFT);
			nk_label(gui->ctx, _l("Enter first point"), NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, _l("Set the reflection line"), NK_TEXT_LEFT);
			nk_label(gui->ctx, _l("Enter second point"), NK_TEXT_LEFT);
		}
	}
	return 1;
}