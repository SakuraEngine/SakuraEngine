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
        Runtime = 1 << 0,
        DevTime = 1 << 1,
        Tool = 1 << 2
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
