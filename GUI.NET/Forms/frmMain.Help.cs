using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml;
using Mesen.GUI.Config;

namespace Mesen.GUI.Forms
{
	public partial class frmMain
	{
		private void mnuAbout_Click(object sender, EventArgs e)
		{
			using(frmAbout frm = new frmAbout()) {
				frm.ShowDialog(this);
			}
		}

		private void mnuHelpWindow_Click(object sender, EventArgs e)
		{
			using(frmHelp frm = new frmHelp()) {
				frm.ShowDialog(sender, this);
			}
		}
	}
}
