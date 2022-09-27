using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;

namespace OutageAPI.Controllers
{
    public class OutageController : Controller
    {
        // GET: Outage
        public string Index()
        {
            return "This is my <b>default</b> action...";
        }
        // GET: Outage/Edit
        public ActionResult Edit()
        {
            return View();
        }
    }
}