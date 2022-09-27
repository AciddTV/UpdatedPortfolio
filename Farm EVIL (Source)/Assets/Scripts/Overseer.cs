using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class Overseer : MonoBehaviour
{
    public GameObject cowPen;
    public GameObject beefBuilding;
    public GameObject cow;

    public bool buildmode;
    int objcheck;
    int tutcheck;
    int instructcheck;

    int _i, _minutes, _hours, _days, _week, _iIncome;
    float _amount;
    float _productionRate;

    float _totCowpen;

    public GameObject objList;

    Transform selected;
    RaycastHit hit;

    Ray ray;

    GameObject btnCPen;
    public GameObject tutStartUI;
    public Text Instructions;

    int poschecker;

    public GameObject border;

    public GameObject TODText;
    int buildchecker;
    int pausechecker;
    float booster = 1;


    public enum Events { disease, demand, investment };

    public Events ev;

    public float Karma;

    public Text txtamount;
    public Text txtkarma;

    public GameObject eventPanel;


    // Start is called before the first frame update
    void Start()
    {
        buildmode = false;
        _i = 0;
        _minutes = 0;
        _hours = 0;
        _days = 0;
        _week = 0;
        _totCowpen = 1;
        objcheck = 1;
        objList.SetActive(false);
        Karma = 0;
        btnCPen.SetActive(false);
        tutcheck = 0;
        instructcheck = 0;
        _amount = 0;
        _productionRate = 1;
        buildchecker = 0;

        tutStartUI.SetActive(false);

        border.SetActive(false);

        pausechecker = 0;

        txtamount.text = "";

        booster = 1;

    }

    // Update is called once per frame
    void Update()
    {   
        if (Input.GetKeyUp(KeyCode.E))
        {
            Debug.Log("Currently Selected: " + selected.name);
        }


        ray = Camera.main.ScreenPointToRay(Input.mousePosition);
        if (Physics.Raycast(ray, out hit, 200f) && Input.GetMouseButtonDown(0))
        {
            selected = hit.collider.transform.parent;
            print("Click Tester: " + selected.name);
        }

        if (tutcheck > 0)
        {
            tutStartUI.SetActive(true);
            switch (instructcheck)
            {
                case 0:
                    Instructions.text = "welcome to FarmEVILle, you must be the owner \n" +
                        "of this new farm here. Well why don't we get \n" +
                        "you settled in, huh?";
                    break;
                case 1:
                    Instructions.text = "I'll show you the tricks of the trade, as \n" +
                        "much as you're willing to listen. And there's quite \n" +
                        "a bit to get through, so listen up";
                    break;
                case 2:
                    Instructions.text = "Now, the heart of this farm is the cash, \n" +
                        "and you've gotta keep it rolling in, so that we can \n" +
                        "make MORE money";
                    break;
                case 3:
                    Instructions.text = "Now how do you suppose we make more cash, huh?\n" +
                        "With these sorry creatures, of course! Sure you could do plants and \n" +
                        "a whole bunch of other stuff, but nothing really brings in the \n" +
                        "*MOOLA* like these things.";
                    break;
                case 4:
                    Instructions.text = "Plus, they're pretty damn tasty. Ever have Wagyuu? \n" +
                        "'Course not, look at you. But I tell you, it's the tastiest damn \n" +
                        "animal I've ever eaten.";
                    break;
                case 5:
                    Instructions.text = "First things first, Money makers. It's time to start \n" +
                        "adding some cows in here. See that button on the bottom right, \n" +
                        "the one that says 'Buy Cow'? Go ahead and click that for me";
                    break;
                case 6:
                    Instructions.text = "Good. Notice how another cow appeared next to the first one.\n" +
                        "This is gonna keep happening every time you get another cow. If you find\n" +
                        "this pen getting a bit full, go ahead and get yourself a new one.\n" +
                        "Down there in the bottom left corner, the button that says 'Build Cowpen'";
                    break;
                case 7:
                    Instructions.text = "Now these don't have to be everything's permanent place. You can \n" +
                        "move everything about by clicking and dragging them in build mode. Let's \n" +
                        "enter build mode by pressing that button in the bottom left.";
                    break;
                case 8:
                    Instructions.text = "Now you can move stuff about, but be careful. You can move everything, \n" +
                        "from the buildings, pens, trees, even the cows themselves. Give it a try, \n" +
                        "they love it!";
                    break;
                case 9:
                    Instructions.text = "NOT SO HARD, YOU'RE GOING TO RUIN MY PRODUCT.\n" +
                        "Just be sure to put them back, as long as they're in a pen, \n" +
                        "I really don't care.";
                    break;
                case 10:
                    Instructions.text = "But what's the good of cows when it's the meat that really matters? \n" +
                        "It's time to get the proper facilities to get the meat. I tell you, once this \n" +
                        "gets some momentum we'll really slaughter the competition.";
                    break;
                case 11:
                    Instructions.text = "Now obviously we've gotta have a means to an end. You've gotta \n" +
                        "rake in some money otherwise you're gonna go under. Why don't you check the \n" +
                        "objectives menu in the top right corner.";
                    break;
                case 12:
                    Instructions.text = "So you've got some work to do. But before I go, let me explain\n" +
                        "how your money works. So to make money, you've gotta spend money. The pen and the \n" +
                        "other facilities all cost money, but each pen, building and cow all make you money \n" +
                        "in return.";
                    break;
                case 13:
                    Instructions.text = "The prices of the buildings are all under the objectives in \n" +
                        "the objective menu";
                    break;
                case 14:
                    Instructions.text = "If the resources aren't enough, there's always the option of... \n" +
                        "less savoury options. But keep in mind, do these enough times, people start \n" +
                        "to catch on. And they never let you live it down.";
                    break;
                case 15:
                    Instructions.text = "Use that button on the left side of the screen that says boost \n," +
                        "and you'll get some boosts to your income. Keep your greed in check, you never know \n" +
                        "who's watching";
                    break;
                case 16:
                    Instructions.text = "You'll probably go get the news every few days, and once that comes, \n" +
                        "you'd better watch it. Sometimes good things can happen. You can have someone invest \n" +
                        "in the farm, or talks of new movements in the community can be swirling.";
                    break;
                case 17:
                    Instructions.text = "Well, you seem to know what you're doing. I'll leave you to it, \n" +
                        "but remember: Go out there and make us some money kid";
                    break;
                case 18:
                    tutStartUI.SetActive(false);
                    break;
            }
        }


        TimeKeeper();
        income();
        KarmaKeeper();
    }
    public void KarmaKeeper()
    {
        txtkarma.text = "Karma: " + Karma.ToString();
    }
    public void boost()
    {
        booster += 0.2f;
        Karma -= 50;
    }
    public void Buildmode()
    {
        buildchecker++;

        if (buildchecker % 2 == 0)
        {
            buildmode = false;
            border.SetActive(false);
        }
        else
        {
            buildmode = true;
            border.SetActive(true);
        }

    }
    public void pause()
    {
        pausechecker++;
        if(pausechecker%2 == 0)
        {
            Debug.Log("Game is no longer paused");
        }
        else
        {
            Debug.Log("Game is paused");
        }
    }

    public void nextInstruct()
    {
        instructcheck++;
    }

    public void btnCowPenPress()
    {
        Instantiate(cowPen, new Vector3((34.01f + (poschecker*16)), 9.536743e-07f, 35.99f), Quaternion.Euler(-90f, 180f, 0f));
        poschecker++;
        _totCowpen++;
        Debug.Log("Total amount of cowpens in scene: " + _totCowpen);
    }
    public void addcow()
    {
        //GameObject.Instantiate(cow, selected);
        Instantiate(cow, selected);
    }
    public void btnBeefPress()
    {
        Instantiate(beefBuilding, new Vector3((34.01f + (poschecker*8)), 9.536743e-07f, 35.99f), Quaternion.identity);
        poschecker++;
        Debug.Log(poschecker);
    }
    public void objListShow()
    {
        objcheck++;
        if(objcheck%2 == 0)
        {
            objList.SetActive(true);
        }
        else
        {
            objList.SetActive(false);
        }
    }
    public void karmaDown()
    {
        Karma -= 50;
    }
    public void TutStart()
    {
        tutcheck = 1;
    }



    public void income()
    {
        _iIncome++;

        if(_iIncome == 120)
        {
            Debug.Log("income was activated");
            float increase = _productionRate * 25;
            _amount +=  (25 * _totCowpen * booster);
            _iIncome = 0;
            Debug.Log(_amount);
            txtamount.text = _amount.ToString();
        }
    }


    public void TimeKeeper()
    {

        
        _i++;

        if(_i == 60)
        {
            _minutes += 10;
            _i = 0;
        }
        if(_minutes == 60)
        {
            _hours++;
            Karma += 10;
            _minutes = 0;
            Debug.Log("Hours: " + _hours);
        }
        if(_hours == 24)
        {
            _days++;
            _hours = 0;
            Debug.Log("Days: " + _days);
        }

        if (_days == 2 && _hours == 0 && _minutes == 0 && _i == 0)      //Here is where the random event happens. You'll want to put the newspaper here and then add a button to close it.
        {                                                               //I've added a gameobject called eventpanel that'll act as the newspaper but it's not implemented anywhere
            int r = Random.Range(0, 2);                                 //You'll need to replace it yourself. Good luck, if there's anything you need, call me and I'll help out.
            switch (r)
            {
                case 0:
                    ev = Events.demand;
                    Debug.Log("Demand event has occured");
                    _productionRate *= 1.5f;
                    break;
                case 1:
                    ev = Events.disease;
                    Debug.Log("Disease event has occured");
                    _productionRate /= 3;
                    break;
                case 2:
                    ev = Events.investment;
                    Debug.Log("Investment event has occured");
                    _amount += 500;
                    break;
            }
        }

        if(_days == 7)
        {
            Debug.Log("Game Ended");
        }

    }
}