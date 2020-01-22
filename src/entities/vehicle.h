#pragma once
#include <r2/config.h>
#include <r2/managers/sceneman.h>

class btGeneric6DofConstraint;
class btGeneric6DofSpring2Constraint;

namespace sr2 {
	class bbnd_file;

	class vehicle_entity;
	class vehicle_camera : public scene_entity {
		public:
			vehicle_camera(const mstring& name);
			~vehicle_camera();

			virtual void onInitialize();
			virtual void onUpdate(f32 frameDt, f32 updateDt);

			vehicle_entity* tracking;
	};

	class vehicle_entity : public scene_entity {
		public:
			vehicle_entity(const mstring& name);
			~vehicle_entity();

			virtual void onInitialize();
			virtual void onUpdate(f32 frameDt, f32 updateDt);

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

			btGeneric6DofSpring2Constraint* armConstraints[4];
			btGeneric6DofConstraint* tireConstraints[4];

			f32 steerAngle;
			f32 maxTorque;

			f32 time;
	};
};

