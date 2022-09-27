using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.UIElements;

public class UIProcedures : MonoBehaviour
{
    public bool isObjShown;
    public GameObject objList;
    private void Start()
    {
        isObjShown = false;
    }
    private void Update()
    {
        if (isObjShown == true)
        {
            objList.SetActive(true);
        }
        else
            objList.SetActive(false);


    }
}
