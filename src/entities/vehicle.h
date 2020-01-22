#pragma once
#include <r2/config.h>
#include <r2/managers/sceneman.h>

#include <gamedata/vehicle_props.h>

class btRaycastVehicle;
class btVehicleRaycaster;
class btDefaultVehicleRaycaster;

namespace r2 {
	class audio_source;
};

namespace sr2 {
	class bnd_file;

	class vehicle_entity;
	class vehicle_camera : public scene_entity {
		public:
			vehicle_camera(const mstring& name);
			~vehicle_camera();

			virtual void onInitialize();
			virtual void onUpdate(f32 frameDt, f32 updateDt);

			vehicle_entity* tracking;
			vec2f angles;
	};

	class vehicle_entity : public scene_entity {
		public:
			struct vehicle_state {
				f32 hp_to_engine_force;
				f32 steer_angle;
				f32 rpm;
				f32 engine_force;
				f32 brake_force;
				f32 trans_gear_ratio;
				f32 rear_gear_ratio;
				bool reverse;
				bool on_ground;
			};

			vehicle_entity(const mstring& name);
			~vehicle_entity();

			virtual void willBeDestroyed();
			virtual void onInitialize();
			virtual void onUpdate(f32 frameDt, f32 updateDt);

			void update_engine_sound();

			void move_to(const mat4f& transform);

			vehicle_camera* camera;

			scene_entity* wheels		[4];
			scene_entity* arms			[4];
			scene_entity* shocks		[4];
			scene_entity* shafts		[4];
			scene_entity* sirens		[4];
			scene_entity* exhausts		[2];
			scene_entity* axles			[2];
			scene_entity* breakables	[8];
			scene_entity* variants		[8];
			scene_entity* body;
			scene_entity* engine;
			scene_entity* oil;
			scene_entity* shadow;
			scene_entity* driver;

			btDefaultVehicleRaycaster* rayCaster;
			btRaycastVehicle* vehicle;

			bnd_file* bounds;
			vehicle_props props;
			vehicle_state state;

			r2::audio_source** engine_sounds;

			f32 time;
	};
};

