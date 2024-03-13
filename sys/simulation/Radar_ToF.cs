using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ToF : MonoBehaviour
{

    public float HorizontalFovAngle {get; set;} = 45f;

    public int HorizontalSteps {get; set;} = 4;

    public float VerticalFovAngle {get; set;} = 45f;

    public int VerticalSteps {get; set;} = 4;

    public float MaximumRange {get; set;} = 4f;

    public float MinimumRange{get; set;} = 0.002f;

    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {

    }

    void FixedUpdate()
    {
        Vector3 front = transform.TransformDirection(Vector3.forward);
        RaycastHit hit;

        for (int y = 0; y <= VerticalSteps; y++) {
            for (int x = 0; x <= HorizontalSteps; x++) {
                float y_angle = (y / (float) this.VerticalSteps - 0.5f) * this.VerticalFovAngle;
                float x_angle = (x / (float) this.HorizontalSteps - 0.5f) * this.HorizontalFovAngle;
                Vector3 ray_dir = Quaternion.AngleAxis(x_angle, transform.up) * Quaternion.AngleAxis(y_angle, transform.right) * front;

                int layerMask = ~(1 << 8);
                bool has_hit = Physics.Raycast(transform.position, ray_dir, out hit, Mathf.Infinity, layerMask);

                if (has_hit && hit.distance <= this.MaximumRange) {
                    Debug.DrawRay(transform.position, ray_dir * hit.distance, Color.yellow);
                }
                else if(has_hit) {
                    Debug.DrawRay(transform.position, ray_dir * hit.distance, Color.blue);
                }
                else{
                    Debug.DrawRay(transform.position, ray_dir, Color.white);
                }
            }
        }
    }
}
