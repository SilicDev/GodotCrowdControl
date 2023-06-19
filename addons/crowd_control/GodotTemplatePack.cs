using System;
using System.Collections.Generic;
using CrowdControl.Common;
using JetBrains.Annotations;

namespace CrowdControl.Games.Packs
{
    [UsedImplicitly]
    class GodotTemplate : SimpleTCPPack
    {
        public override string Host => "127.0.0.1";

        public override ushort Port => 58430;

        public GodotTemplate(UserRecord player, Func<CrowdControlBlock, bool> responseHandler, Action<object> statusUpdateHandler) : base(player, responseHandler, statusUpdateHandler)
        {
        }

        public override Game Game => new("GodotTemplate", "GodotTemplate", "PC", ConnectorType.SimpleTCPServerConnector);

        public override EffectList Effects
        {
            get
            {
                List<Effect> effects = new List<Effect>
                {
                    new Effect("Demo", "demo") { Duration = 30 }
                };
                return effects;
            }
        }
    }
}