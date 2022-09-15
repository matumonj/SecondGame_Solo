#include "Cllision.h"//collision

bool Cllision::ChenkSphere2Sphere(XMFLOAT3 object1, XMFLOAT3& object2,float r)
{
	//‰~“–‚½‚è”»’è
		float a = object1.x - object2.x;
		float b = object1.y - object2.y;
		float c = object1.z - object2.z;
		float distance = sqrtf(a * a + b * b + c * c);
		//int radius = R1;
		return distance<=r;

}
