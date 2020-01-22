#pragma once
#include <r2/managers/memman.h>
#include <json.hpp>

namespace sr2 {
	class gamedata_manager;
	class vehicle_entity;

	#pragma pack(push, 1)
	struct vehicle_props {
		struct physics_info {
			bool exists;
			r2::i32 audio_id;
			r2::vec3f size;
			r2::vec3f center_of_gravity;
			r2::i32 number_of_glows;
			r2::f32 mass;
			r2::f32 elasticity;
			r2::f32 friction;
			r2::f32 impulse_limit_2;
			r2::i32 spin_axis;
			r2::i32 flash;
			r2::i32 number_of_parts;

			struct {
				r2::vec3f position;
				r2::vec3f position_variation;
				r2::vec3f velocity;
				r2::vec3f velocity_variation;
				r2::f32 lifespan;
				r2::f32 mass;
				r2::f32 mass_variation;
				r2::f32 radius;
				r2::f32 radius_variation;
				r2::f32 drag;
				r2::f32 drag_variation;
				r2::f32 d_radius;
				r2::f32 d_radius_variation;
				r2::i32 d_alpha;
				r2::i32 d_alpha_variation;
				r2::i32 d_rotation;
				r2::i32 d_rotation_variation;
				r2::i32 initial_blast;
				r2::f32 spawn_rate;
				r2::f32 spawn_duration;
				r2::f32 gravity;
				r2::i32 tex_frame_start;
				r2::i32 tex_frame_end;
				r2::i32 birth_flags;
				r2::i32 color_index;
			} birth_rule;

			r2::i32 tex_number;
			r2::i32 bill_flags;
			r2::f32 y_radius;
			r2::i32 collider_id;
			r2::i32 collision_primitive;
			r2::i32 collision_type;
		};

		struct camera_info {
			r2::f32 blend_time;
			r2::f32 blend_goal;
			r2::f32 near_plane;
			r2::f32 far_plane;
			r2::f32 field_of_view;
			r2::f32 pitch;
			r2::f32 pov_jitter_amp;
			r2::f32 look_at;
			r2::f32 look_above;
			r2::f32 min_dist;
			r2::f32 max_dist;
			r2::i32 approach_on;
			r2::f32 app_rot;
			r2::f32 app_rot_min;
			r2::f32 app_x_rot;
			r2::f32 app_y_pos;
			r2::f32 app_xz_pos;
			r2::f32 app_xz_pos_min;
			r2::f32 app_xz_pos_max;
			r2::f32 app_pos_min;
			r2::f32 app_app;
			r2::i32 app_app_on;
			r2::f32 hill_min;
			r2::f32 hill_max;
			r2::f32 hill_lerp;
			r2::vec3f offset;
			r2::vec3f reverse_offset;
			r2::vec3f track_to;
			r2::i32 collide_type;
			r2::i32 min_max_on;
			r2::i32 track_break;
			r2::f32 min_speed;
			r2::f32 max_speed;
			r2::f32 app_inc;
			r2::f32 app_dec;
			r2::f32 min_hard_steer;
			r2::f32 drift_delay;
			r2::f32 vertical_offset;
			r2::f32 front_rate;
			r2::f32 rear_rate;
			r2::f32 flip_delay;
			r2::i32 steer_on;
			r2::f32 steer_min;
			r2::f32 steer_amount;
			r2::i32 reverse_on;
			r2::f32 reverse_delay;
			r2::f32 reverse_on_app;
			r2::f32 reverse_off_app;

			// used by UI vehicle views, I think
			r2::f32 polar_height;
			r2::f32 polar_distance;
			r2::f32 polar_azimuth;
			r2::f32 polar_incline;
			r2::f32 polar_delta;
			r2::i32 azimuth_lock;
		};

		struct engine_sample {
			struct volume_fade {
				r2::f32 start_volume;
				r2::f32 end_volume;
				r2::f32 fade_in_start_rpm;
				r2::f32 fade_in_end_rpm;
				r2::f32 fade_out_start_rpm;
				r2::f32 fade_out_end_rpm;
			};

			struct pitch_shift {
				r2::f32 start_pitch;
				r2::f32 end_pitch;
				r2::f32 start_rpm;
				r2::f32 end_rpm;
			};

			r2::mstring file_name;
			r2::i32 volume_fade_count;
			volume_fade* volume_faders;
			r2::i32 pitch_shift_count;
			pitch_shift* pitch_shifters;
		};

		vehicle_props();

		void destroy();

		struct {
			r2::i32 double_pivot;
			r2::i32 mirror_pivot;
			r2::f32 impact_threshold;
			r2::f32 max_damage;
			r2::f32 med_damage;
			r2::f32 regenerate_rate;
			r2::vec3f smoke_offset;
			r2::f32 textel_damage_radius;
		} damage;

		struct {
			struct {
				r2::vec3f ang_c_damp;
				r2::vec3f ang_vel_2_damp;
				r2::f32 down;
				r2::f32 drag;
			} aero;

			struct {
				r2::f32 buoyancy;
				r2::f32 damping;
			} fluid;

			struct {
				r2::f32 damp_coef;
				r2::f32 torque_coef;
			} rear_axel;

			struct {
				r2::f32 damp_coef;
				r2::f32 torque_coef;
			} front_axel;

			struct {
				r2::f32 angular_inertia;
				r2::f32 brake_static_coef;
			} freetrain;

			struct {
				r2::f32 brake_static_coef;
			} drivetrain;

			struct {
				r2::f32 boost_duration;
				r2::f32 boost_horsepower;
				r2::f32 gcl;
				r2::f32 max_horsepower;
				r2::f32 idle_rpm;
				r2::f32 max_rpm;
				r2::f32 optimum_rpm;
			} engine;

			struct {
				r2::f32 downshift_bias_max;
				r2::f32 upshift_bias;
				r2::f32 gear_change_time;
				r2::f32 high;
				r2::f32 low;
			} transmission;

			struct {
				r2::f32 brake_coef;
				r2::f32 camber_limit;
				r2::f32 handbrake_coef;
				r2::f32 optimum_slip_percent;
				r2::f32 sliding_friction;
				r2::f32 suspension_damping;
				r2::f32 suspension_extent;
				r2::f32 suspension_factor;
				r2::f32 suspension_limit;
				r2::f32 lateral_damping;
				r2::f32 lateral_disp_limit;
				r2::f32 longitudinal_drag;
				r2::f32 longitudinal_disp_limit;
			} rear_wheels;

			struct {
				r2::f32 brake_coef;
				r2::f32 camber_limit;
				r2::f32 handbrake_coef;
				r2::f32 optimum_slip_percent;
				r2::f32 sliding_friction;
				r2::f32 steering_limit;
				r2::f32 steering_offset;
				r2::f32 suspension_damping;
				r2::f32 suspension_extent;
				r2::f32 suspension_factor;
				r2::f32 suspension_limit;
				r2::f32 lateral_damping;
				r2::f32 lateral_disp_limit;
				r2::f32 longitudinal_drag;
				r2::f32 longitudinal_disp_limit;
			} front_wheels;

			r2::f32 mass;
			r2::f32 air_gravity;
			r2::i32 drivetrain_type;
			r2::f32 bound_elasticity;
			r2::f32 sss_threshold;
			r2::f32 sss_value;
			r2::vec3f inertia_box;
			r2::vec3f model_offset;
			r2::vec3f center_of_mass;
		} sim;

		struct {
			r2::f32 drift;
			r2::f32 drift_thrust;
			r2::f32 pitch;
			r2::f32 reverse_180;
			r2::f32 roll;
			r2::f32 spin_180;
			r2::f32 turn;
			r2::f32 wheelie;
		} gyro;

		struct {
			r2::f32 motion_threshold;
			r2::f32 position_threshold;
			r2::f32 rotation;
			r2::f32 time_threshold;
			r2::f32 translation;
			r2::f32 turn;
		} stuck;

		struct {
			camera_info near;
			camera_info far;
			camera_info pov;
		} camera;

		struct {
			physics_info wheels		[4];
			physics_info arms		[4];
			physics_info shocks		[4];
			physics_info shafts		[4];
			physics_info sirens		[4];
			physics_info exhausts	[2];
			physics_info axles		[2];
			physics_info breakables	[8];
			physics_info variants	[8];
			physics_info engine;
			physics_info oil;
			physics_info driver;
		} physics;

		struct {
			r2::i32 engine_sample_count;
			engine_sample* engine_samples;
		} sound;
	};
	#pragma pack(pop)

	class vehicle_props_loader {
		public:
			vehicle_props_loader();
			~vehicle_props_loader();

			bool load(gamedata_manager* gdm, vehicle_entity* vehicle, const r2::mstring& vehicleName);

			void props(vehicle_entity* vehicle);

			nlohmann::json jsonProps;
	};
};