#include "body.h"
#include "collision.h"
#include "forces.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "shape.h"
#include "state.h"
#include "test_util.h"
#include <emscripten.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const double GRAVITY1 = 500;
const double GRAVITY2 = 90;
const double TURTLE_GRAVITY = 150;
const size_t NUM_BONES = 36;
const size_t NUM_DIFF_BONES = 3;
const size_t NUM_PINEAPPLES = 1;
const size_t INIT_NUM_HOPPERS = 3;
const size_t NUM_SHELVES = 12;
const size_t NUM_SUB_WINDOWS = 4;
const size_t INIT_NUM_TURTLES = 10;
const size_t SPAWN_TIME = 100;

const double MARKER_LENGTH = 5;
const double MARKER_VERTICES = 10;
const double LILY_PAD_LENGTH = 50;
const double NO_TURTLE_RADIUS = 100;
const double NO_GOLDEN_BONE_RADIUS = 25;
const double NO_BONE_RADIUS = 75;

const rgb_color_t BLACK = {0.0, 0.0, 0.0};
const rgb_color_t BLUE = {0.0, 0.0, 1.0};

const rgb_color_t OPENING = {255.0 / 255.0, 251.0 / 255.0, 227.0 / 255.0};
const rgb_color_t LEVEL_1_INSTRUCTIONS = {163.0 / 255.0, 200.0 / 255.0,
                                          255.0 / 255.0};
const rgb_color_t LEVEL_2_INSTRUCTIONS = {217.0 / 255.0, 148.0 / 255.0,
                                          148.0 / 255.0};
const rgb_color_t LEVEL_3_INSTRUCTIONS = {175.0 / 255.0, 218.0 / 255.0,
                                          175.0 / 255.0};

const rgb_color_t LOSING = {176.0 / 255.0, 193.0 / 255.0, 219.0 / 255.0};
const rgb_color_t WINNING = {229.0 / 255.0, 196.0 / 255.0, 214.0 / 255.0};

const rgb_color_t LEVEL_1 = {232.0 / 255.0, 232.0 / 255.0, 232.0 / 255.0};
const rgb_color_t LEVEL_2 = {219.0 / 255.0, 138.0 / 255.0, 138.0 / 255.0};
const rgb_color_t LEVEL_3_LILY_PAD = {102.0 / 255.0, 125.0 / 255.0,
                                      102.0 / 255.0};
const rgb_color_t LEVEL_3_GRASS = {155.0 / 255.0, 215.0 / 255.0, 156.0 / 255.0};
const rgb_color_t LEVEL_3_POND = {200.0 / 250.0, 215.0 / 255.0, 245.0 / 255.0};

const size_t HOPPER_MASS_2 = 10;
const size_t HOPPER_MASS = 1000000;
const size_t BONE_MASS = 5;
const size_t PINEAPPLE_MASS = 10;
const size_t SHELF_MASS = INFINITY;
const size_t PAD_MASS = 100;
const size_t TURTLE_MASS = 10;

const double BONE_SCORE = 10.0;
const double GOLDEN_BONE_SCORE = 20.0;
const double PINEAPPLE_SCORE = 100.0;
const double PORTAL_SCORE = 200.0;
const double DECOY_BONE_SCORE = -50.0;
const double TURTLE_SCORE = 100;

const double GROUND_CR = 1.0;
const double MIN_ELASTICITY = 0;
const double MAX_ELASTICITY = 10;
const double WALL_WIDTH = 10.0;
const double WALL_ELASTICITY = 1.0;
const double ELASTICITY_STEP = 0.05;
const double POSITION_STEP = 10;
const double ANGLE_STEP = 0.3;
const double COOLDOWN_TIME = 3;

const double OPENING_LEVEL = 0;
const double LEVEL1_RULES = 0.5;
const double LEVEL1 = 1;
const double LEVEL2_RULES = 1.5;
const double LEVEL2 = 2;
const double LEVEL3_RULES = 2.5;
const double LEVEL3 = 3;
const double FAIL = 3.5;
const double WIN = 4;

const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const double HALF_MULTIPLY = 0.5;
const double QUARTER_MULTIPLY = 0.25;
const double THIRD_MULTIPLY = 1.0 / 3.0;
const double TWO_THIRD_MULTIPLY = 2.0 / 3.0;
const size_t DOUBLE = 2;
const size_t REMAINDER_2 = 2;
const size_t REMAINDER_3 = 3;
const size_t REMAINDER_4 = 4;

const vector_t PORTAL_DIMENSIONS = (vector_t){.x = 20, .y = 100};
const vector_t HOPPER_SIZE = (vector_t){50, 50};
const vector_t BONE_SIZE = (vector_t){.x = 40, .y = 40};
const vector_t PINEAPPLE_SIZE = (vector_t){.x = 50, .y = 50};
const vector_t SHELF_SIZE = (vector_t){.x = 100, .y = 20};
const vector_t TURTLE_SIZE = (vector_t){.x = 50, .y = 50};
const double LILY_PIC_DIM = 3;

const vector_t PORTAL_VELOCITY = (vector_t){.x = 0, .y = 100};
const vector_t HOPPER_VELOCITY = (vector_t){.x = 300, .y = 100};
const vector_t HOPPER_VELOCITY_2 = (vector_t){.x = 0, .y = 100};
const vector_t PROJECTILE_VELOCITY = (vector_t){.x = 100, .y = 100};
const vector_t MAX_VEL = (vector_t){.x = 200, .y = 200};

const size_t HOPPER_IDX = 1;
const size_t HOPPER_IDX_3 = 2;
const size_t GROUND_IDX = 2;
const size_t PORTAL_IDX_1 = 3;
const size_t LILY_PAD_IDX = 1;
const size_t GOLDEN_BONE_IDX = 3;
const size_t PINEAPPLE_BOMB_IDX = 4;

const size_t PROJECTILE_POINTS_MASS = 10;
const size_t PROJECTILE_LENGTH = 15;

typedef struct state {
  scene_t *scene;
  bool level_passed;
  size_t hoppers_left;
  bool projectile;
  double score;
  double active_level;
  double time_passed;
  double time_since_death;
  bool cooldown_active;
  bool pineapple_state;
} state_t;

typedef struct status {
  bool hopper_status;
  bool golden_bone_status;
  bool pineapple_status;
  bool portal_status;
} status_t;

// checks if any special bodies are in the scene still
status_t check_status(scene_t *curr_scene) {
  bool hopper_tracker = 0;
  bool golden_bone_tracker = 0;
  bool pineapple_tracker = 0;
  bool portal_tracker = 0;
  for (size_t i = 0; i < scene_bodies(curr_scene); i++) {
    char *info = body_get_info(scene_get_body(curr_scene, i));
    if (!strcmp(info, "Hopper")) {
      hopper_tracker = 1;
    } else if (!strcmp(info, "Golden Bone")) {
      golden_bone_tracker = 1;
    } else if (!strcmp(info, "Pineapple")) {
      pineapple_tracker = 1;
    } else if (!strcmp(info, "Portal")) {
      portal_tracker = 1;
    }
  }
  return (status_t){.hopper_status = hopper_tracker,
                    .golden_bone_status = golden_bone_tracker,
                    .pineapple_status = pineapple_tracker,
                    .portal_status = portal_tracker};
}

list_t *make_hopper_shape() {
  return make_rectangle(HOPPER_SIZE.y, HOPPER_SIZE.x,
                        HOPPER_SIZE.x * HALF_MULTIPLY,
                        HOPPER_SIZE.y * HALF_MULTIPLY);
}

void populate_background(scene_t *scene, char *img_path) {
  list_t *bg_shape = make_rectangle(
      WINDOW.y, WINDOW.x, WINDOW.x * HALF_MULTIPLY, WINDOW.y * HALF_MULTIPLY);
  body_t *bg = body_init_with_info(bg_shape, 1, BLACK, "Background", NULL);
  body_set_img_texture(bg, img_path);
  body_set_dimensions(bg, (vector_t){WINDOW.x, WINDOW.y});
  scene_add_body(scene, bg);
}

void populate_hopper(scene_t *scene, rgb_color_t color) {
  // add hopper to scene bodies
  list_t *hopper_shape = make_hopper_shape();
  body_t *hopper =
      body_init_with_info(hopper_shape, HOPPER_MASS, color, "Hopper", NULL);
  body_set_score(hopper, 0.0);
  body_set_dimensions(hopper, HOPPER_SIZE);
  body_set_elasticity(hopper, GROUND_CR);
  body_set_img_texture(hopper, "for_images/hopper.png");
  scene_add_body(scene, hopper);
}

void populate_transition_scene(state_t *state, bool success) {
  srand(time(NULL));
  scene_t *curr_scene = state->scene;

  // background at index 0
  if (success) {
    populate_background(curr_scene, "for_images/Winning_Screen_FINAL.png");
  } else {
    populate_background(curr_scene, "for_images/Losing_Screen_FINAL.png");
  }

  // hopper at index 1
  populate_hopper(curr_scene, LOSING);
  body_t *hopper = scene_get_body(curr_scene, HOPPER_IDX);
  if (state->active_level == LEVEL3) {
    hopper = scene_get_body(curr_scene, HOPPER_IDX_3 + 1);
  }
  vector_t lower_centre = {WINDOW.x * HALF_MULTIPLY,
                           HOPPER_SIZE.y * HALF_MULTIPLY};
  body_set_centroid(hopper, lower_centre);
  if (success) {
    body_set_color(hopper, WINNING);
  }
}

void end_init(state_t *curr_state) {
  scene_free(curr_state->scene);
  scene_t *end_scene = scene_init();
  curr_state->scene = end_scene;
  curr_state->level_passed = true;
  curr_state->active_level = WIN;
  populate_transition_scene(curr_state, 1);
}

void fail_init(state_t *curr_state) {
  scene_free(curr_state->scene);
  scene_t *end_scene = scene_init();
  curr_state->scene = end_scene;
  curr_state->level_passed = false;
  curr_state->active_level = FAIL;
  populate_transition_scene(curr_state, 0);
}

// makes hopper travel in projectile motion
void projectile_motion(state_t *state, double dt) {
  body_t *body = scene_get_body(state->scene, HOPPER_IDX);
  list_t *vertices = body_get_shape(body);
  vector_t distance = vec_multiply(dt, body_get_velocity(body));
  vector_t curr_vel = body_get_velocity(body);
  for (size_t i = 0; i < list_size(vertices); i++) {
    vector_t *vector = list_get(vertices, i);
    double y = vector->y + distance.y;
    if ((y < 0) && (check_status(state->scene).portal_status)) {
      state->hoppers_left = state->hoppers_left - 1;
      state->time_since_death = 0;
      state->cooldown_active = true;
      // if there are no more tries, then the player has failed
      if (state->hoppers_left == 0) {
        fail_init(state);
        break;
      } else {
        body_set_velocity(body, VEC_ZERO);
        body_set_centroid(body, (vector_t){HOPPER_SIZE.x * HALF_MULTIPLY,
                                           HOPPER_SIZE.y * HALF_MULTIPLY});
        state->projectile = false;
        break;
      }
    } else {
      body_set_velocity(body,
                        (vector_t){curr_vel.x, (curr_vel.y - GRAVITY2 * dt)});
    }
  }
}

void add_score(state_t *state, body_t *player, body_t *collided_body) {
  list_t *player_shape = body_get_actual_shape(player);
  list_t *collided_shape = body_get_actual_shape(collided_body);
  if ((collided_shape != NULL) && (player_shape != NULL)) {
    if (get_collision_bool(find_collision(player_shape, collided_shape))) {
      state->score = state->score + body_get_score(collided_body);
    }
  }
}

void add_score_level3(state_t *state) {
  status_t status = check_status(state->scene);
  if (!status.golden_bone_status) {
    state->score += GOLDEN_BONE_SCORE;
  }
  if ((!status.pineapple_status) && (state->pineapple_state == 1)) {
    state->score += PINEAPPLE_SCORE;
    state->pineapple_state = 0;
  }
  if (!status.hopper_status) {
    fail_init(state);
  } else {
    for (size_t i = PINEAPPLE_BOMB_IDX + 1; i < scene_bodies(state->scene);
         i++) {
      body_t *body = scene_get_body(state->scene, i);
      list_t *projectile;
      if (!strcmp(body_get_info(body), "Brick Projectile")) {
        projectile = body_get_actual_shape(body);

        for (size_t j = PINEAPPLE_BOMB_IDX + 1; j < scene_bodies(state->scene);
             j++) {
          body_t *body2 = scene_get_body(state->scene, j);
          if (!strcmp(body_get_info(body2), "Turtle")) {
            list_t *turtle_shape = body_get_actual_shape(body2);
            if (get_collision_bool(find_collision(projectile, turtle_shape))) {
              state->score = state->score + TURTLE_SCORE;
            }
          }
        }
      }
    }
  }
}

void show_best_path(list_t *pos_list, scene_t *scene) {
  for (size_t i = 0; i < list_size(pos_list); i++) {
    list_t *shape = make_star(MARKER_LENGTH, MARKER_VERTICES,
                              ((vector_t *)list_get(pos_list, i))->x,
                              ((vector_t *)list_get(pos_list, i))->y);
    body_t *marker = body_init_with_info(shape, 1, BLACK, "Marker", NULL);
    scene_add_body(scene, marker);
  }
}

list_t *calculate_bone_positions_1(scene_t *scene, size_t num_bones) {
  list_t *pos_list = list_init(1, free);
  double dt = 1.0;
  for (size_t i = 0; i < num_bones; i++) {
    vector_t *s = malloc(sizeof(vector_t));
    s->y = (HOPPER_VELOCITY.y * dt) - (HALF_MULTIPLY * GRAVITY1 * dt);
    s->x = (HOPPER_VELOCITY.x * dt);
    dt++;
    list_add(pos_list, s);
  }
  return pos_list;
}

list_t *calculate_bone_positions_2(scene_t *scene, size_t num_bones) {
  list_t *pos_list = list_init(1, free);
  double dt = 0.0;
  for (size_t i = 0; i < num_bones; i++) {
    vector_t *s = malloc(sizeof(vector_t));
    s->y = WINDOW.y * HALF_MULTIPLY + (HOPPER_VELOCITY.y * dt) -
           (HALF_MULTIPLY * GRAVITY2 * dt * dt);
    s->x = (HOPPER_VELOCITY.x * dt);
    dt += HALF_MULTIPLY;
    list_add(pos_list, s);
  }
  return pos_list;
}

list_t *calculate_bone_positions_rand(scene_t *scene, size_t num_bones) {
  list_t *pos_list = list_init(1, free);
  for (size_t i = 0; i < num_bones; i++) {
    vector_t *pos = malloc(sizeof(vector_t));
    *pos = (vector_t){(rand() % (int)WINDOW.x), (rand() % (int)WINDOW.y)};
    while (((pos->x < NO_BONE_RADIUS) && (pos->y < NO_BONE_RADIUS))) {
      *pos = (vector_t){(rand() % (int)WINDOW.x), (rand() % (int)WINDOW.y)};
    }
    list_add(pos_list, pos);
  }
  return pos_list;
}

vector_t calculate_pineapple_position(scene_t *scene) {
  time_t t;
  srand((unsigned)time(&t));
  return (vector_t){(rand() % (int)(WINDOW.x * HALF_MULTIPLY)) +
                        DOUBLE * NO_GOLDEN_BONE_RADIUS,
                    (rand() % (int)(WINDOW.y * HALF_MULTIPLY)) +
                        DOUBLE * NO_GOLDEN_BONE_RADIUS};
}

list_t *make_bone_shape() {
  return make_rectangle(BONE_SIZE.x * QUARTER_MULTIPLY,
                        BONE_SIZE.y * HALF_MULTIPLY, 0, 0);
}

list_t *make_pineapple_shape() {
  return make_rectangle(PINEAPPLE_SIZE.y, PINEAPPLE_SIZE.x, 0, 0);
}

list_t *make_portal_shape() {
  return make_rectangle(PORTAL_DIMENSIONS.y, PORTAL_DIMENSIONS.x,
                        WINDOW.x - PORTAL_DIMENSIONS.y * HALF_MULTIPLY,
                        WINDOW.y * HALF_MULTIPLY);
}

void populate_bones_list(scene_t *scene, size_t num_bones, rgb_color_t color) {
  list_t *positions = list_init(NUM_DIFF_BONES, NULL);
  list_add(positions,
           calculate_bone_positions_1(scene, NUM_BONES * QUARTER_MULTIPLY));
  list_add(positions,
           calculate_bone_positions_2(scene, NUM_BONES * QUARTER_MULTIPLY));
  list_add(positions,
           calculate_bone_positions_rand(scene, NUM_BONES * HALF_MULTIPLY));

  positions = list_merge(positions);

  body_t *hopper = scene_get_body(scene, HOPPER_IDX);
  for (size_t i = 0; i < list_size(positions); i++) {
    list_t *bone_shape = make_bone_shape();
    body_t *bone =
        body_init_with_info(bone_shape, BONE_MASS, color, "Bone", NULL);

    // the first half of the bones are golden bones
    if (i > list_size(positions) * HALF_MULTIPLY) {
      body_set_score(bone, GOLDEN_BONE_SCORE);

      // the second half of the bones are normal bones
    } else {
      body_set_score(bone, BONE_SCORE);
    }
    body_set_centroid(bone, *(vector_t *)list_get(positions, i));
    body_set_dimensions(bone, BONE_SIZE);
    body_set_img_texture(bone, "for_images/bone.png");
    scene_add_body(scene, bone);
    create_one_destructive_collision(scene, hopper, bone);
  }
  free(positions);
}

void populate_pineapple_list(scene_t *scene, size_t num_pineapples,
                             rgb_color_t color, size_t hopper_index) {
  body_t *hopper = scene_get_body(scene, hopper_index);
  for (size_t i = 0; i < num_pineapples; i++) {
    vector_t pineapple_position = calculate_pineapple_position(scene);
    list_t *pineapple_shape = make_pineapple_shape();
    body_t *pineapple = body_init_with_info(pineapple_shape, PINEAPPLE_MASS,
                                            color, "Pineapple", NULL);
    body_set_centroid(pineapple, pineapple_position);
    body_set_score(pineapple, PINEAPPLE_SCORE);
    body_set_dimensions(pineapple, PINEAPPLE_SIZE);
    body_set_img_texture(pineapple, "for_images/Pineapple.png");
    scene_add_body(scene, pineapple);
    create_one_destructive_collision(scene, hopper, pineapple);
  }
}

vector_t calculate_pineapple_position3(scene_t *scene) {
  vector_t pos = (vector_t){(rand() % (int)WINDOW.x), (rand() % (int)WINDOW.y)};
  while (((pos.x > WINDOW.x * HALF_MULTIPLY - NO_GOLDEN_BONE_RADIUS) &&
          (pos.x < WINDOW.x * HALF_MULTIPLY + NO_GOLDEN_BONE_RADIUS)) ||
         ((pos.y > WINDOW.y * HALF_MULTIPLY - NO_GOLDEN_BONE_RADIUS) &&
          (pos.y < WINDOW.y * HALF_MULTIPLY + NO_GOLDEN_BONE_RADIUS))) {
    pos = (vector_t){(rand() % (int)WINDOW.x), (rand() % (int)WINDOW.y)};
  }
  return pos;
}

void populate_ground(scene_t *curr_scene) {
  list_t *ground_shape =
      make_rectangle(WALL_WIDTH, WINDOW.x, WINDOW.x * HALF_MULTIPLY, 0);
  body_t *ground =
      body_init_with_info(ground_shape, INFINITY, BLACK, "Ground", NULL);
  body_set_centroid(ground, (vector_t){WINDOW.x * HALF_MULTIPLY, 0});

  scene_add_body(curr_scene, ground);
}

void populate_portal(scene_t *scene) {
  list_t *portal = make_portal_shape();

  body_t *to_add = body_init_with_info(portal, INFINITY, BLACK, "Portal", NULL);
  body_set_centroid(to_add,
                    (vector_t){WINDOW.x - PORTAL_DIMENSIONS.x * HALF_MULTIPLY,
                               WINDOW.y * HALF_MULTIPLY});
  body_set_velocity(to_add, PORTAL_VELOCITY);
  body_set_score(to_add, PORTAL_SCORE);
  body_set_dimensions(to_add, PORTAL_DIMENSIONS);
  body_set_img_texture(to_add, "for_images/portal.png");
  scene_add_body(scene, to_add);
  body_t *hopper = scene_get_body(scene, HOPPER_IDX);
  create_one_destructive_collision(scene, hopper, to_add);
}

void portal_motion(state_t *state, size_t portal_idx, double dt) {
  scene_t *scene = state->scene;
  body_t *portal = scene_get_body(scene, portal_idx);
  list_t *vertices = body_get_shape(portal);
  vector_t curr_vel = body_get_velocity(portal);
  for (size_t i = 0; i < list_size(vertices); i++) {
    vector_t distance = vec_multiply(dt, curr_vel);
    vector_t *vertex = list_get(vertices, i);
    double y = vertex->y + distance.y;
    size_t changed = 0;
    curr_vel = body_get_velocity(portal);
    if (y <= 0 || y >= WINDOW.y) {
      vector_t new_vel = vec_negate(curr_vel);
      body_set_velocity(portal, new_vel);
      changed = 1;
    }
    if (changed == 1) {
      break;
    }
  }
}

void on_key1(char key, key_event_type_t type, double held_time,
             state_t *state) {
  scene_t *scene = state->scene;
  body_t *player = scene_get_body(scene, HOPPER_IDX);
  if (type == KEY_PRESSED) {
    switch (key) {
    case T:
      emscripten_force_exit(0);
    case DOWN_ARROW:
      body_set_velocity(player, (vector_t){0, -HOPPER_VELOCITY.y});
      break;
    case UP_ARROW:
      body_set_velocity(player, (vector_t){0, HOPPER_VELOCITY.y});
      break;
    case SPACE:
      if (state->projectile == false) {
        body_set_velocity(player, HOPPER_VELOCITY);
        state->projectile = true;
      }
    }
  } else if (type == KEY_RELEASED) {
    body_set_velocity(player, VEC_ZERO);
  }
}

// POPULATING SCENE INIT
void populate_scene1_init(scene_t *curr_scene) {
  // background at index 0
  populate_background(curr_scene, "for_images/Level_1_Background_FINAL.png");

  // player at index 1
  populate_hopper(curr_scene, LEVEL_1);

  // ground at index 2
  populate_ground(curr_scene);

  // portal at index 3
  populate_portal(curr_scene);

  // pineapple at index 4
  populate_pineapple_list(curr_scene, NUM_PINEAPPLES, LEVEL_1, HOPPER_IDX);

  // bones at index 5 onwards
  populate_bones_list(curr_scene, NUM_BONES, LEVEL_1);
}

// calculating the positions of the shelves for level 2
list_t *calculate_shelf_positions(scene_t *scene, size_t num_shelves) {
  time_t t;
  srand((unsigned)time(&t));

  list_t *pos = list_init(num_shelves, NULL);
  // list of the y positions
  double y_pos_top = (rand() % (int)(WINDOW.y * THIRD_MULTIPLY)) +
                     (int)(WINDOW.y * TWO_THIRD_MULTIPLY);
  double y_pos_mid = (rand() % (int)(WINDOW.y * THIRD_MULTIPLY)) +
                     (int)WINDOW.y * THIRD_MULTIPLY;
  double y_pos_bottom = (rand() % (int)(WINDOW.y * THIRD_MULTIPLY));
  for (size_t i = 0; i < num_shelves; i++) {
    double x_pos = (rand() % ((int)WINDOW.x / NUM_SUB_WINDOWS)) +
                   ((i % REMAINDER_4) * (int)WINDOW.x / NUM_SUB_WINDOWS);
    vector_t *posit = malloc(sizeof(vector_t));
    // populate a third of the shelves in the bottom y third
    if (i < num_shelves * THIRD_MULTIPLY) {
      *posit = (vector_t){x_pos, y_pos_bottom};
      list_add(pos, posit);
    }
    // populate a thid the shelves in the top y third
    else if ((i >= num_shelves * THIRD_MULTIPLY) &&
             (i < num_shelves * TWO_THIRD_MULTIPLY)) {
      *posit = (vector_t){x_pos, y_pos_mid};
      list_add(pos, posit);
      // a third of the shelves are populated in random locations
    } else {
      *posit = (vector_t){x_pos, y_pos_top};
      list_add(pos, posit);
    }
  }
  return pos;
}

void populate_shelves(state_t *state, size_t num_shelves) {
  scene_t *scene = state->scene;
  body_t *hopper = scene_get_body(scene, HOPPER_IDX);
  double hopper_cr = body_get_elasticity(hopper);
  list_t *shelf_positions = calculate_shelf_positions(scene, num_shelves);
  for (size_t i = 0; i < num_shelves; i++) {
    list_t *shelf_shape = make_rectangle(SHELF_SIZE.y, SHELF_SIZE.x, 0, 0);
    body_t *shelf =
        body_init_with_info(shelf_shape, SHELF_MASS, BLACK, "Shelf", NULL);
    body_set_score(shelf, 0);
    body_set_centroid(shelf, *(vector_t *)list_get(shelf_positions, i));
    body_set_dimensions(shelf, SHELF_SIZE);
    scene_add_body(scene, shelf);

    // 4 shelves are breakable
    if (i % REMAINDER_3 == 0) {
      create_physics_collision(scene, hopper_cr, hopper, shelf);
      create_one_destructive_collision(scene, hopper, shelf);
    }
    // 4 shelves are rotatable (there are 6 but 2 of them also have one-sided
    // destructive collisions)
    else if (i % REMAINDER_2 == 0) {
      create_physics_collision(scene, hopper_cr, hopper, shelf);
      create_rotating_collision(scene, hopper, shelf);
    }
    // the other shelves have normal physics collisions
    else {
      create_physics_collision(scene, hopper_cr, hopper, shelf);
    }
  }
}

list_t *calculate_bone_positions_shelf(scene_t *scene,
                                       list_t *shelf_positions) {
  list_t *bone_pos = list_init(1, NULL);
  for (size_t i = 0; i < list_size(shelf_positions); i += 1) {
    vector_t *shelf_pos = list_get(shelf_positions, i);
    vector_t shelf_top =
        vec_add(*shelf_pos, (vector_t){0, SHELF_SIZE.y * HALF_MULTIPLY});
    vector_t *bone_position = malloc(sizeof(vector_t));
    *bone_position =
        vec_add(shelf_top, (vector_t){0, BONE_SIZE.y * HALF_MULTIPLY});
    list_add(bone_pos, bone_position);
  }
  return bone_pos;
}

void populate_bones2_list(scene_t *scene, size_t num_bones, rgb_color_t color) {
  list_t *positions = list_init(DOUBLE, NULL);
  list_add(positions,
           calculate_bone_positions_rand(scene, num_bones * HALF_MULTIPLY));

  // 12 (NUM_SHELVES) bones on the shelves
  list_add(positions,
           calculate_bone_positions_shelf(
               scene, calculate_shelf_positions(scene, NUM_SHELVES)));

  positions = list_merge(positions);

  body_t *hopper = scene_get_body(scene, HOPPER_IDX);
  for (size_t i = 0; i < list_size(positions); i++) {
    list_t *bone_shape = make_bone_shape();
    body_t *bone =
        body_init_with_info(bone_shape, BONE_MASS, color, "Bone", NULL);

    // all of the positions except 4 will be normal bones
    // one bone is a golden bone
    if (i % NUM_BONES == 0) {
      body_set_score(bone, GOLDEN_BONE_SCORE);
      body_set_img_texture(bone, "for_images/golden_bone.png");
      body_set_info(bone, "Golden Bone");

      // 3 bones are decoy bones
    } else if (i % (int)(NUM_BONES * QUARTER_MULTIPLY) == 0) {
      body_set_score(bone, DECOY_BONE_SCORE);
      body_set_img_texture(bone, "for_images/decoy_bone.png");
      body_set_info(bone, "Decoy Bone");
    }
    // the other bones will be normal
    else {
      body_set_img_texture(bone, "for_images/bone.png");
      body_set_score(bone, BONE_SCORE);
      body_set_info(bone, "Bone");
    }

    // golden bone should be accessible to the player
    body_set_centroid(bone, *(vector_t *)list_get(positions, i));
    if (!strcmp(body_get_info(bone), "Golden Bone")) {
      body_set_centroid(bone,
                        (vector_t){rand() % (int)(WINDOW.x),
                                   rand() % (int)(WINDOW.y * HALF_MULTIPLY)});
    }
    body_set_dimensions(bone, BONE_SIZE);
    scene_add_body(scene, bone);
    create_one_destructive_collision(scene, hopper, bone);
  }
  free(positions);
}

void hopper_bounce(state_t *state, double dt) {
  body_t *hopper = scene_get_body(state->scene, HOPPER_IDX);
  list_t *vertices = body_get_shape(hopper);
  vector_t curr_vel = body_get_velocity(hopper);
  for (size_t i = 0; i < list_size(vertices); i++) {
    vector_t distance = vec_multiply(dt, curr_vel);
    vector_t *vertex = list_get(vertices, i);
    double y = vertex->y + distance.y;
    size_t changed = 0;
    vector_t curr_vel = body_get_velocity(hopper);
    if (y <= 0) {
      vector_t new_vel =
          (vector_t){curr_vel.x, curr_vel.y * -1 * body_get_elasticity(hopper)};
      if ((fabs(new_vel.x) <= MAX_VEL.x) && (fabs(new_vel.y) <= MAX_VEL.y)) {
        body_set_velocity(hopper, new_vel);
        changed = 1;
        state->projectile = false;
      }
    } else if (y >= WINDOW.y) {
      fail_init(state);
      break;
    } else {
      body_set_velocity(hopper,
                        (vector_t){curr_vel.x, (curr_vel.y - GRAVITY2 * dt)});
    }
    if (changed == 1) {
      break;
    }
  }
}

void on_key2(char key, key_event_type_t type, double held_time,
             state_t *state) {
  scene_t *scene = state->scene;
  body_t *player = scene_get_body(scene, HOPPER_IDX);
  double curr_elasticity = body_get_elasticity(player);
  vector_t curr_position = body_get_centroid(player);
  double set_elasticity = curr_elasticity;

  if (type == KEY_PRESSED) {
    if (!check_status(scene).pineapple_status) {
      switch (key) {
      case DOWN_ARROW:
        set_elasticity = curr_elasticity - (ELASTICITY_STEP * held_time);
        if (set_elasticity > MIN_ELASTICITY) {
          body_set_elasticity(player, set_elasticity);
        }
        break;
      case UP_ARROW:
        set_elasticity = curr_elasticity + (ELASTICITY_STEP * held_time * 20);
        if (set_elasticity < MAX_ELASTICITY) {
          body_set_elasticity(player, set_elasticity);
        }
        break;
      }
    }
    switch (key) {
    case LEFT_ARROW:
      body_set_centroid(player,
                        (vector_t){curr_position.x - POSITION_STEP * held_time,
                                   curr_position.y});
      break;
    case RIGHT_ARROW:
      body_set_centroid(player,
                        (vector_t){curr_position.x + POSITION_STEP * held_time,
                                   curr_position.y});
      break;
    }
  }
}

void populate_scene2_init(state_t *curr_state) {
  scene_t *curr_scene = curr_state->scene;
  // background at index 0
  populate_background(curr_scene, "for_images/Level_2_Background_FINAL.png");

  // player at index 1
  populate_hopper(curr_scene, LEVEL_2);
  // body_set_color(scene_get_body(curr_scene, HOPPER_IDX), BONE_COLOR_2);
  body_t *hopper = scene_get_body(curr_scene, HOPPER_IDX);
  body_set_centroid(hopper, (vector_t){HOPPER_SIZE.x * HALF_MULTIPLY,
                                       WINDOW.y * HALF_MULTIPLY});
  body_set_velocity(hopper, HOPPER_VELOCITY_2);
  body_set_mass(hopper, HOPPER_MASS_2);

  // ground at index 2
  populate_ground(curr_scene);
  body_t *ground = scene_get_body(curr_scene, GROUND_IDX);
  create_physics_collision(curr_scene, GROUND_CR, hopper, ground);

  // pineapple at index 3
  populate_pineapple_list(curr_scene, NUM_PINEAPPLES, LEVEL_2, HOPPER_IDX);

  // shelves from index 4 onwards
  populate_shelves(curr_state, NUM_SHELVES);

  // bones after shelves
  populate_bones2_list(curr_scene, NUM_BONES, LEVEL_2);
}

void populate_lily_pad(scene_t *curr_scene) {
  list_t *shape = make_pacman(LILY_PAD_LENGTH, WINDOW.x * HALF_MULTIPLY,
                              WINDOW.y * HALF_MULTIPLY);
  body_t *lily_pad =
      body_init_with_info(shape, PAD_MASS, LEVEL_3_POND, "Lily Pad", NULL);
  body_set_img_texture(lily_pad, "for_images/lily_pad.png");
  body_set_dimensions(lily_pad, (vector_t){LILY_PAD_LENGTH * LILY_PIC_DIM,
                                           LILY_PAD_LENGTH * LILY_PIC_DIM});
  scene_add_body(curr_scene, lily_pad);
}

// spawns the turtles in random locations
// applies newtonian gravity to the turtles with Hopper
// creates destructive collision with hopper

void populate_turtles(scene_t *scene, rgb_color_t color) {
  srand(time(NULL));

  body_t *hopper = scene_get_body(scene, HOPPER_IDX_3);
  list_t *turtle_shape =
      make_rectangle(TURTLE_SIZE.y, TURTLE_SIZE.y, WINDOW.x * HALF_MULTIPLY,
                     WINDOW.y * HALF_MULTIPLY);
  body_t *turtle =
      body_init_with_info(turtle_shape, TURTLE_MASS, color, "Turtle", NULL);
  body_set_dimensions(turtle, TURTLE_SIZE);

  // make sure the turtles are far enough from hopper initially
  vector_t pos = (vector_t){(rand() % (int)WINDOW.x), (rand() % (int)WINDOW.y)};
  while (((pos.x > WINDOW.x * HALF_MULTIPLY - NO_TURTLE_RADIUS) &&
          (pos.x < WINDOW.x * HALF_MULTIPLY + NO_TURTLE_RADIUS)) ||
         ((pos.y > WINDOW.y * HALF_MULTIPLY - NO_TURTLE_RADIUS) &&
          (pos.y < WINDOW.y * HALF_MULTIPLY + NO_TURTLE_RADIUS))) {
    pos = (vector_t){(rand() % (int)WINDOW.x), (rand() % (int)WINDOW.y)};
  }
  body_set_centroid(turtle, pos);
  if (pos.x <= WINDOW.x * HALF_MULTIPLY) {
    body_set_img_texture(turtle, "for_images/Turtle_Left.png");
  } else {
    body_set_img_texture(turtle, "for_images/Turtle_Right.png");
  }
  body_set_score(turtle, TURTLE_SCORE);
  scene_add_body(scene, turtle);
  create_newtonian_gravity(scene, TURTLE_GRAVITY,
                           scene_get_body(scene, LILY_PAD_IDX), turtle);
  create_destructive_collision(scene, turtle, hopper);
}

void populate_brick_projectile(scene_t *curr_scene) {
  list_t *projectile_shape =
      make_star(PROJECTILE_LENGTH, PROJECTILE_POINTS_MASS,
                WINDOW.x * HALF_MULTIPLY, WINDOW.y * HALF_MULTIPLY);
  body_t *hopper = scene_get_body(curr_scene, HOPPER_IDX_3);

  body_t *projectile =
      body_init_with_info(projectile_shape, PROJECTILE_POINTS_MASS, BLACK,
                          "Brick Projectile", NULL);
  body_set_velocity(projectile, PROJECTILE_VELOCITY);
  body_set_centroid(projectile, body_get_centroid(hopper));
  scene_add_body(curr_scene, projectile);

  for (size_t i = 0; i < scene_bodies(curr_scene); i++) {
    body_t *turtle = scene_get_body(curr_scene, i);
    if (!strcmp(body_get_info(turtle), "Turtle")) {
      create_destructive_collision(curr_scene, projectile, turtle);
    }
    create_destructive_collision(curr_scene, projectile,
                                 scene_get_body(curr_scene, GOLDEN_BONE_IDX));
    create_destructive_collision(
        curr_scene, projectile, scene_get_body(curr_scene, PINEAPPLE_BOMB_IDX));
  }
}

void populate_golden_bone(scene_t *curr_scene, rgb_color_t color) {
  time_t t;
  srand((unsigned)time(&t));

  list_t *bone_shape =
      make_rectangle(BONE_SIZE.y, BONE_SIZE.y, WINDOW.x * HALF_MULTIPLY,
                     WINDOW.y * HALF_MULTIPLY);
  body_t *bone =
      body_init_with_info(bone_shape, BONE_MASS, color, "Golden Bone", NULL);
  body_set_img_texture(bone, "for_images/golden_bone.png");
  body_set_dimensions(bone, BONE_SIZE);

  vector_t pos = (vector_t){(rand() % (int)WINDOW.x), (rand() % (int)WINDOW.y)};
  while (((pos.x > WINDOW.x * HALF_MULTIPLY - NO_GOLDEN_BONE_RADIUS) &&
          (pos.x < WINDOW.x * HALF_MULTIPLY + NO_GOLDEN_BONE_RADIUS)) ||
         ((pos.y > WINDOW.y * HALF_MULTIPLY - NO_GOLDEN_BONE_RADIUS) &&
          (pos.y < WINDOW.y * HALF_MULTIPLY + NO_GOLDEN_BONE_RADIUS))) {
    pos = (vector_t){(rand() % (int)WINDOW.x), (rand() % (int)WINDOW.y)};
  }

  body_set_centroid(bone, pos);
  body_set_score(bone, GOLDEN_BONE_SCORE);

  scene_add_body(curr_scene, bone);
}

void pineapple_bomb(state_t *curr_state) {
  scene_t *curr_scene = curr_state->scene;
  if (curr_state->pineapple_state) {
    for (size_t i = 0; i < scene_bodies(curr_scene); i++) {
      body_t *body = scene_get_body(curr_scene, i);
      if (!strcmp(body_get_info(body), "Turtle")) {
        body_remove(body);
      }
    }
  }
}

void on_key3(char key, key_event_type_t type, double held_time,
             state_t *state) {
  scene_t *scene = state->scene;
  body_t *lily_pad = scene_get_body(scene, LILY_PAD_IDX);
  double curr_angle = body_get_rotation(lily_pad);
  if (type == KEY_PRESSED) {
    switch (key) {
    case LEFT_ARROW:
      body_set_rotation(lily_pad, (curr_angle + held_time * ANGLE_STEP));
      body_set_img_texture(lily_pad, "for_images/lily_pad.png");
      break;
    case RIGHT_ARROW:
      body_set_rotation(lily_pad, (curr_angle - held_time * ANGLE_STEP));
      body_set_img_texture(lily_pad, "for_images/lily_pad.png");
      break;
    case SPACE:
      populate_brick_projectile(scene);
      body_t *projectile = scene_get_body(scene, scene_bodies(scene) - 1);
      vector_t velocity = {
          PROJECTILE_VELOCITY.x * cos(body_get_rotation(lily_pad)),
          PROJECTILE_VELOCITY.y * sin(body_get_rotation(lily_pad))};
      body_set_velocity(projectile, velocity);
    }
  }
}

void populate_scene3_init(state_t *state) {
  scene_t *curr_scene = state->scene;

  // background at index 0
  populate_background(curr_scene, "for_images/Level_3_Background_FINAL.png");

  // lily pad at index 1
  populate_lily_pad(curr_scene);

  // hopper at index 2
  populate_hopper(curr_scene, LEVEL_3_LILY_PAD);

  body_t *hopper = scene_get_body(curr_scene, HOPPER_IDX_3);
  body_set_centroid(
      hopper, (vector_t){WINDOW.x * HALF_MULTIPLY, WINDOW.y * HALF_MULTIPLY});
  body_set_velocity(hopper, VEC_ZERO);

  // golden bone at index 3
  populate_golden_bone(curr_scene, LEVEL_3_GRASS);

  // pineapple at index 4
  populate_pineapple_list(curr_scene, NUM_PINEAPPLES, LEVEL_3_GRASS,
                          HOPPER_IDX_3);
  body_t *pineapple = scene_get_body(curr_scene, PINEAPPLE_BOMB_IDX);
  body_set_centroid(pineapple, calculate_pineapple_position3(curr_scene));

  // the next INIT_NUM_TURTLES(5) are turtles, from 5-8
  for (size_t i = 0; i < INIT_NUM_TURTLES; i++) {
    populate_turtles(curr_scene, LEVEL_3_GRASS);
  }
}

void level1_init(state_t *curr_state) {
  scene_free(curr_state->scene);
  scene_t *level1_scene = scene_init();
  curr_state->scene = level1_scene;
  populate_scene1_init(level1_scene);
  curr_state->level_passed = false;
  curr_state->hoppers_left = INIT_NUM_HOPPERS;
  curr_state->score = 0.0;
  curr_state->projectile = false;
  curr_state->active_level = LEVEL1;
  curr_state->time_passed = 0;
  curr_state->time_since_death = 0;
  curr_state->cooldown_active = false;
  sdl_on_key((void *)on_key1);
}

void on_key_transition_1(char key, key_event_type_t type, double held_time,
                         state_t *state) {
  if (type == KEY_PRESSED) {
    switch (key) {
    case T:
      level1_init(state);
    }
  }
}

void level1_rules(state_t *curr_state) {
  scene_free(curr_state->scene);
  scene_t *level1 = scene_init();
  curr_state->scene = level1;
  populate_background(level1, "for_images/Level_1_Instructions_FINAL.png");
  populate_hopper(level1, LEVEL_1_INSTRUCTIONS);
  curr_state->active_level = LEVEL1_RULES;
  sdl_on_key((void *)on_key_transition_1);
}

void on_key_transition_0(char key, key_event_type_t type, double held_time,
                         state_t *state) {
  if (type == KEY_PRESSED) {
    switch (key) {
    case T:
      level1_rules(state);
    }
  }
}

void opening_init(state_t *curr_state) {
  scene_t *opening_scene = scene_init();
  curr_state->scene = opening_scene;

  populate_background(opening_scene, "for_images/Opening_FINAL.png");
  populate_hopper(opening_scene, OPENING);
  curr_state->active_level = OPENING_LEVEL;
  sdl_on_key((void *)on_key_transition_0);
}

void level2_init(state_t *curr_state) {
  scene_free(curr_state->scene);
  scene_t *level2_scene = scene_init();
  curr_state->scene = level2_scene;
  curr_state->level_passed = false;
  curr_state->hoppers_left = 1;
  curr_state->projectile = false;
  curr_state->active_level = LEVEL2;
  populate_scene2_init(curr_state);
  hopper_bounce(curr_state, time_since_last_tick());
  sdl_on_key((void *)on_key2);
}

void on_key_transition_2(char key, key_event_type_t type, double held_time,
                         state_t *state) {
  if (type == KEY_PRESSED) {
    switch (key) {
    case T:
      level2_init(state);
    }
  }
}

void level2_rules(state_t *curr_state) {
  scene_free(curr_state->scene);
  scene_t *level2 = scene_init();
  curr_state->scene = level2;
  populate_background(level2, "for_images/Level_2_Instructions_FINAL.png");
  populate_hopper(level2, LEVEL_2_INSTRUCTIONS);
  curr_state->active_level = LEVEL2_RULES;
  sdl_on_key((void *)on_key_transition_2);
}

void level3_init(state_t *curr_state) {
  scene_free(curr_state->scene);
  scene_t *level3_scene = scene_init();
  curr_state->scene = level3_scene;
  curr_state->level_passed = false;
  curr_state->active_level = LEVEL3;
  curr_state->pineapple_state = 1;
  populate_scene3_init(curr_state);
  sdl_on_key((void *)on_key3);
}

void on_key_transition_3(char key, key_event_type_t type, double held_time,
                         state_t *state) {
  if (type == KEY_PRESSED) {
    switch (key) {
    case T:
      level3_init(state);
    }
  }
}

void level3_rules(state_t *curr_state) {
  scene_free(curr_state->scene);
  scene_t *level3 = scene_init();
  curr_state->scene = level3;
  curr_state->active_level = LEVEL3_RULES;
  populate_background(level3, "for_images/Level_3_Instructions_FINAL.png");
  populate_hopper(level3, LEVEL_3_INSTRUCTIONS);
  sdl_on_key((void *)on_key_transition_3);
}

state_t *emscripten_init() {
  sdl_init(VEC_ZERO, WINDOW);
  state_t *new_state = malloc(sizeof(state_t));
  opening_init(new_state);
  return new_state;
}

bool check_pass(state_t *state, size_t portal_idx) {
  if (scene_get_body(state->scene, portal_idx) == NULL) {
    return 1;
  }
  scene_t *curr_scene = state->scene;
  body_t *hopper = scene_get_body(curr_scene, HOPPER_IDX);
  if (state->active_level == LEVEL3) {
    hopper = scene_get_body(curr_scene, HOPPER_IDX_3);
  }
  list_t *hopper_shape = body_get_actual_shape(hopper);
  body_t *portal = scene_get_body(curr_scene, portal_idx);
  list_t *portal_shape = body_get_actual_shape(portal);
  if (get_collision_bool(find_collision(hopper_shape, portal_shape)) == 1) {
    state->level_passed = 1;
  } else {
    state->level_passed = 0;
  }
  return state->level_passed;
}

void wrap_around1(scene_t *scene) {
  body_t *hopper = scene_get_body(scene, HOPPER_IDX);
  double curr_centre_x = body_get_centroid(hopper).x;
  double curr_centre_y = body_get_centroid(hopper).y;
  if (curr_centre_y > (WINDOW.y - HOPPER_SIZE.y * HALF_MULTIPLY)) {
    body_set_centroid(hopper,
                      (vector_t){curr_centre_x, HOPPER_SIZE.y * HALF_MULTIPLY});
  } else if (curr_centre_y < (HOPPER_SIZE.y * HALF_MULTIPLY)) {
    body_set_centroid(hopper,
                      (vector_t){curr_centre_x,
                                 (WINDOW.y - (HOPPER_SIZE.y * HALF_MULTIPLY))});
  }
}

void emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  scene_t *curr_scene = state->scene;

  if (state->active_level == LEVEL1 || state->active_level == LEVEL2 ||
      state->active_level == LEVEL3) {
    body_t *hopper = scene_get_body(curr_scene, HOPPER_IDX);

    if (state->active_level == LEVEL3) {
      hopper = scene_get_body(curr_scene, HOPPER_IDX_3);
    }

    scene_tick(curr_scene, dt);
    state->time_passed++;

    // if Hopper passes through the portal, transition to the next level
    // for level 1, if the pineapple is eaten, show the best path
    if ((state->active_level) == LEVEL1) {
      if (check_pass(state, PORTAL_IDX_1)) {
        level2_rules(state);
        state->active_level = LEVEL2_RULES;
        curr_scene = state->scene;
        hopper = scene_get_body(curr_scene, HOPPER_IDX);
      } else {
        wrap_around1(curr_scene);
        if (!check_status(curr_scene).pineapple_status) {
          show_best_path(calculate_bone_positions_2(
                             curr_scene, NUM_BONES * QUARTER_MULTIPLY),
                         curr_scene);
        }
        if (state->time_since_death < COOLDOWN_TIME && state->cooldown_active) {
          body_set_centroid(hopper,
                            (vector_t){HOPPER_SIZE.x / 2, HOPPER_SIZE.y / 2});
          state->time_since_death = state->time_since_death + dt;
          // state->hoppers_left = state->hoppers_left + 1;
          state->projectile = false;
        } else {
          state->cooldown_active = false;
        }
        portal_motion(state, PORTAL_IDX_1, dt);
      }
    }

    // for level 2, if the golden bone has been eaten, spawn the portal
    else if ((state->active_level) == LEVEL2) {
      if (check_pass(state, scene_bodies(curr_scene) - 1)) {
        level3_rules(state);
        state->active_level = LEVEL3_RULES;
        curr_scene = state->scene;
        hopper = scene_get_body(curr_scene, HOPPER_IDX);
      } else {
        hopper_bounce(state, dt);
        body_t *potential_portal =
            scene_get_body(curr_scene, scene_bodies(curr_scene) - 1);
        if ((!check_status(curr_scene).golden_bone_status) &&
            (strcmp(body_get_info(potential_portal), "Portal"))) {
          populate_portal(curr_scene);
          body_t *portal =
              scene_get_body(curr_scene, scene_bodies(curr_scene) - 1);
          body_set_rotation(portal, M_PI * HALF_MULTIPLY);
          vector_t portal_centroid =
              (vector_t){WINDOW.x - PORTAL_DIMENSIONS.y * HALF_MULTIPLY,
                         PORTAL_DIMENSIONS.x};
          body_set_centroid(portal, portal_centroid);
        }
        if (!strcmp(body_get_info(potential_portal), "Portal")) {
          portal_motion(state, scene_bodies(curr_scene) - 1, dt);
        }
        emscripten_log(EM_LOG_NO_PATHS, "Coefficient of restitution: %.2f",
                       body_get_elasticity(hopper));
      }
    }

    // turtles spawn randomly
    else if ((state->active_level) == LEVEL3) {
      double time_passed = state->time_passed;
      scene_t *curr_scene = state->scene;
      hopper = scene_get_body(curr_scene, HOPPER_IDX_3);
      body_t *lily_pad = scene_get_body(curr_scene, LILY_PAD_IDX);
      body_set_velocity(lily_pad, VEC_ZERO);

      // every spawn_time seconds, spawn a new turtle
      if ((int)time_passed % (int)SPAWN_TIME == 1) {
        populate_turtles(curr_scene, LEVEL_3_GRASS);
      }

      if (!check_status(curr_scene).pineapple_status) {
        pineapple_bomb(state);
        state->pineapple_state = 0;
      }

      status_t status = check_status(curr_scene);
      // if the golden bone is destroyed, then the game is won
      if (!status.golden_bone_status) {
        end_init(state);
        state->level_passed = 1;
      }

      // if Hopper no longer exists, the player has failed
      else if (!status.hopper_status) {
        fail_init(state);
        state->level_passed = 0;
      }
    }

    // loops through all that hopper can destructively collide with, and adds
    // the scores to the state
    // if (check_status(curr_scene).hopper_status) {
    if (state->level_passed == 0) {
      for (size_t i = 1; i < scene_bodies(curr_scene); i++) {
        if (scene_get_body(curr_scene, i) != NULL) {
          if (state->active_level >= LEVEL3) {
            add_score_level3(state);
          } else {
            add_score(state, hopper, scene_get_body(curr_scene, i));
          }
        }
      }

      if (state->projectile == true) {
        projectile_motion(state, dt);
      }
    }
  }
  sdl_render_scene(state);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
