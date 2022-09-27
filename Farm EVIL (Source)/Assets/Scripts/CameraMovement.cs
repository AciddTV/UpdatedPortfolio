using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;

public class CameraMovement : MonoBehaviour, IPointerDownHandler, IPointerUpHandler
{
    public GameObject cam;
    Vector3 newvec = new Vector3(0f, 0f, 1f);
    public float movement;
    // Start is called before the first frame update
    void Start()
    {

    }

    // Update is called once per frame
    void Update()
    {
        if (Input.GetKey(KeyCode.W))
        {
            cam.transform.Translate(Vector3.forward * movement * Time.deltaTime, Space.World);
        }
        if (Input.GetKey(KeyCode.A))
        {
            cam.transform.Translate(Vector3.left * movement * Time.deltaTime, Space.World);
        }
        if (Input.GetKey(KeyCode.S))
        {
            cam.transform.Translate(Vector3.back * movement * Time.deltaTime, Space.World);
        }
        if (Input.GetKey(KeyCode.D))
        {
            cam.transform.Translate(Vector3.right * movement * Time.deltaTime, Space.World);
        }

    }
    public void Left()
    {
        cam.transform.Translate(Vector3.left * movement * Time.deltaTime, Space.World);
    }
    public void Right()
    {
        cam.transform.Translate(Vector3.right * movement * Time.deltaTime, Space.World);
    }
    public void Forward()
    {
        cam.transform.Translate(Vector3.forward * movement * Time.deltaTime, Space.World);
    }
    public void Backward()
    {
        cam.transform.Translate(Vector3.back * movement * Time.deltaTime, Space.World);
    }

    public void OnPointerDown(PointerEventData eventData)
    {
        buttonpressed = true;
    }

    public void OnPointerUp(PointerEventData eventData)
    {
        buttonpressed = false;
    }

    public bool buttonpressed;

}
