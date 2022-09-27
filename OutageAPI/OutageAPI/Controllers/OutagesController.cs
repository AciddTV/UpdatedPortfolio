using System;
using System.Collections.Generic;
using System.Data;
using System.Data.Entity;
using System.Linq;
using System.Net;
using System.Web;
using System.Web.Mvc;
using OutageAPI.Models;
using System.Text.Json;
using System.Threading.Tasks;
using PublicHoliday;

namespace OutageAPI.Controllers
{
    public class OutagesController : Controller
    {
        // public List<string> areas = new List<string>();
        public OutageDBContext db = new OutageDBContext();

        // GET: Outages
        public ActionResult Index()
        {
            return View(db.outages.ToList());
        }

        // GET: Outages/Details/5
        public ActionResult Details(int? id)
        {
            if (id == null)
            {
                return new HttpStatusCodeResult(HttpStatusCode.BadRequest);
            }
            Outage outage = db.outages.Find(id);
            if (outage == null)
            {
                return HttpNotFound();
            }
            return View(outage);
        }

        // GET: Outages/Create
        public ActionResult Create()
        {
            return View();
        }

        // POST: Outages/Create
        // To protect from overposting attacks, enable the specific properties you want to bind to, for 
        // more details see https://go.microsoft.com/fwlink/?LinkId=317598.
        [HttpPost]
        [ValidateAntiForgeryToken]
        public ActionResult Create([Bind(Include = "ID,Area,Provider,LeadTime")] Outage outage)
        {
            if (ModelState.IsValid)
            {
                db.outages.Add(outage);
                db.SaveChanges();
                return RedirectToAction("Index");
            }

            return View(outage);
        }

        // GET: Outages/Edit/5
        public ActionResult Edit(int? id)
        {
            if (id == null)
            {
                return new HttpStatusCodeResult(HttpStatusCode.BadRequest);
            }
            Outage outage = db.outages.Find(id);
            if (outage == null)
            {
                return HttpNotFound();
            }
            return View(outage);
        }

        // POST: Outages/Edit/5
        // To protect from overposting attacks, enable the specific properties you want to bind to, for 
        // more details see https://go.microsoft.com/fwlink/?LinkId=317598.
        [HttpPost]
        [ValidateAntiForgeryToken]
        public ActionResult Edit([Bind(Include = "ID,Area,Provider,LeadTime")] Outage outage)
        {
            if (ModelState.IsValid)
            {
                db.Entry(outage).State = EntityState.Modified;
                db.SaveChanges();
                return RedirectToAction("Index");
            }
            return View(outage);
        }

        // GET: Outages/Delete/5
        public ActionResult Delete(int? id)
        {
            if (id == null)
            {
                return new HttpStatusCodeResult(HttpStatusCode.BadRequest);
            }
            Outage outage = db.outages.Find(id);
            if (outage == null)
            {
                return HttpNotFound();
            }
            return View(outage);
        }

        // POST: Outages/Delete/5
        [HttpPost, ActionName("Delete")]
        [ValidateAntiForgeryToken] 
        public ActionResult DeleteConfirmed(int id)
        {
            Outage outage = db.outages.Find(id);
            db.outages.Remove(outage);
            db.SaveChanges();
            return RedirectToAction("Index");
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                db.Dispose();
            }
            base.Dispose(disposing);
        }
        public JsonResult Json()
        {
            var outages = from m in db.outages
                          select m.Area;
            return Json(outages.ToList(), JsonRequestBehavior.AllowGet);
        }
        public JsonResult JsonProviders()
        {
            var outages = from m in db.outages
                          select m.Provider;
            return Json(outages.ToList(), JsonRequestBehavior.AllowGet);
        }
        public JsonResult JsonLeads()
        {
            var outages = from m in db.outages
                          select m.LeadTime;
            return Json(outages.ToList(), JsonRequestBehavior.AllowGet);
        }

        DateTime getBusinessDays(DateTime startdate, int days)
        {
            var date = startdate;
            while(days > 0)
            {
                date = date.AddDays(1);
                if(date.DayOfWeek == DayOfWeek.Saturday)
                {
                    date = date.AddDays(2);
                }
                if(date.DayOfWeek == DayOfWeek.Sunday)
                {
                    date = date.AddDays(1);
                }
                bool isholiday = new SouthAfricaPublicHoliday().IsPublicHoliday(date);
                if (isholiday)
                {
                    date = date.AddDays(1);
                }
                days--;
            }
            return date;
        }

        public JsonResult GetBDays()
        {
            var now = DateTime.Now;
            now = getBusinessDays(DateTime.Now, 21);
            return Json($"{now.Year}/{now.Month}/{now.Day}", JsonRequestBehavior.AllowGet);
        }
    }
}
