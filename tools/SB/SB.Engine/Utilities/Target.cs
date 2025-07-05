using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SB
{
    [Flags]
    public enum TargetCategory
    {
        None = 0,
        Package = 1 << 0,
        Runtime = 1 << 1,
        DevTime = 1 << 2,
        Tests = 1 << 3,
        Tool = 1 << 3
    };

    [AttributeUsage(AttributeTargets.Class)]
    public class TargetScript : Attribute
    {
        public TargetScript(TargetCategory category = TargetCategory.Runtime)
        {
            Category = category;
        }
        public TargetCategory Category { get; }
    }
}
