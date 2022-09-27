using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Data.Entity;
using System.ComponentModel.DataAnnotations.Schema;
using System.ComponentModel.DataAnnotations;

namespace OutageAPI.Models
{
    public class Outage
    {
        [Key]
        public int ID { get; set; }
        public string Area { get; set; }
        public string Provider { get; set; }
        public string LeadTime { get; set; }
    }
    public class OutageDBContext : DbContext
    { 
        public DbSet<Outage> outages { get; set; }
    }

}