using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DragObject : MonoBehaviour
{
    private Vector3 moffset;
    private float mzCoord;
    public Overseer ov;

    private void OnMouseDown()
    {
        mzCoord = Camera.main.WorldToScreenPoint(gameObject.transform.position).z;
        moffset = gameObject.transform.position - getMouseWorldPos();
    }

    private Vector3 getMouseWorldPos()
    {
        Vector3 mousePoint = Input.mousePosition;

        mousePoint.z = mzCoord;

        return Camera.main.ScreenToWorldPoint(mousePoint);
    }

    private void OnMouseDrag()
    {
        if (ov.buildmode == true)
        {
            transform.position = getMouseWorldPos() + moffset;
        }
        else
        {
            Debug.Log("Not in build mode");
        }

    }
}
