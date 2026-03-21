namespace Mesen.GUI.Forms.Cheats
{
	partial class frmCheat
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if(disposing && (components != null)) {
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
            this.label2 = new System.Windows.Forms.Label();
            this.btnBrowse = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.txtCheatName = new System.Windows.Forms.TextBox();
            this.grpCode = new System.Windows.Forms.GroupBox();
            this.tlpAdd = new System.Windows.Forms.TableLayoutPanel();
            this.radCustom = new System.Windows.Forms.RadioButton();
            this.txtProActionRocky = new System.Windows.Forms.TextBox();
            this.txtGameGenie = new System.Windows.Forms.TextBox();
            this.radGameGenie = new System.Windows.Forms.RadioButton();
            this.radProActionRocky = new System.Windows.Forms.RadioButton();
            this.tlpCustom = new System.Windows.Forms.TableLayoutPanel();
            this.lblAddress = new System.Windows.Forms.Label();
            this.txtAddress = new System.Windows.Forms.TextBox();
            this.lblNewValue = new System.Windows.Forms.Label();
            this.txtValue = new System.Windows.Forms.TextBox();
            this.txtCompare = new System.Windows.Forms.TextBox();
            this.chkCompareValue = new System.Windows.Forms.CheckBox();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.radAbsoluteAddress = new System.Windows.Forms.RadioButton();
            this.radRelativeAddress = new System.Windows.Forms.RadioButton();
            this.chkEnabled = new System.Windows.Forms.CheckBox();
            this.txtGameName = new System.Windows.Forms.TextBox();
            this.tableLayoutPanel2.SuspendLayout();
            this.grpCode.SuspendLayout();
            this.tlpAdd.SuspendLayout();
            this.tlpCustom.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // baseConfigPanel
            // 
            this.baseConfigPanel.Location = new System.Drawing.Point(0, 272);
            this.baseConfigPanel.Size = new System.Drawing.Size(385, 29);
            // 
            // tableLayoutPanel2
            // 
            this.tableLayoutPanel2.ColumnCount = 3;
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel2.Controls.Add(this.label2, 0, 0);
            this.tableLayoutPanel2.Controls.Add(this.btnBrowse, 2, 0);
            this.tableLayoutPanel2.Controls.Add(this.label1, 0, 1);
            this.tableLayoutPanel2.Controls.Add(this.txtCheatName, 1, 1);
            this.tableLayoutPanel2.Controls.Add(this.grpCode, 0, 3);
            this.tableLayoutPanel2.Controls.Add(this.chkEnabled, 0, 2);
            this.tableLayoutPanel2.Controls.Add(this.txtGameName, 1, 0);
            this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel2.Location = new System.Drawing.Point(0, 0);
            this.tableLayoutPanel2.Name = "tableLayoutPanel2";
            this.tableLayoutPanel2.RowCount = 4;
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel2.Size = new System.Drawing.Size(385, 272);
            this.tableLayoutPanel2.TabIndex = 3;
            // 
            // label2
            // 
            this.label2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(3, 8);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(35, 12);
            this.label2.TabIndex = 4;
            this.label2.Text = "Game:";
            // 
            // btnBrowse
            // 
            this.btnBrowse.AutoSize = true;
            this.btnBrowse.Location = new System.Drawing.Point(313, 3);
            this.btnBrowse.Name = "btnBrowse";
            this.btnBrowse.Size = new System.Drawing.Size(69, 23);
            this.btnBrowse.TabIndex = 1;
            this.btnBrowse.Text = "Browse...";
            this.btnBrowse.UseVisualStyleBackColor = true;
            this.btnBrowse.Click += new System.EventHandler(this.btnBrowse_Click);
            // 
            // label1
            // 
            this.label1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 36);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(71, 12);
            this.label1.TabIndex = 1;
            this.label1.Text = "Cheat Name:";
            // 
            // txtCheatName
            // 
            this.tableLayoutPanel2.SetColumnSpan(this.txtCheatName, 2);
            this.txtCheatName.Dock = System.Windows.Forms.DockStyle.Fill;
            this.txtCheatName.Location = new System.Drawing.Point(80, 32);
            this.txtCheatName.MaxLength = 255;
            this.txtCheatName.Name = "txtCheatName";
            this.txtCheatName.Size = new System.Drawing.Size(302, 21);
            this.txtCheatName.TabIndex = 2;
            // 
            // grpCode
            // 
            this.tableLayoutPanel2.SetColumnSpan(this.grpCode, 3);
            this.grpCode.Controls.Add(this.tlpAdd);
            this.grpCode.Dock = System.Windows.Forms.DockStyle.Fill;
            this.grpCode.Location = new System.Drawing.Point(3, 81);
            this.grpCode.Name = "grpCode";
            this.grpCode.Size = new System.Drawing.Size(379, 188);
            this.grpCode.TabIndex = 3;
            this.grpCode.TabStop = false;
            this.grpCode.Text = "Code";
            // 
            // tlpAdd
            // 
            this.tlpAdd.ColumnCount = 2;
            this.tlpAdd.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tlpAdd.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tlpAdd.Controls.Add(this.radCustom, 0, 2);
            this.tlpAdd.Controls.Add(this.txtProActionRocky, 1, 1);
            this.tlpAdd.Controls.Add(this.txtGameGenie, 1, 0);
            this.tlpAdd.Controls.Add(this.radGameGenie, 0, 0);
            this.tlpAdd.Controls.Add(this.radProActionRocky, 0, 1);
            this.tlpAdd.Controls.Add(this.tlpCustom, 1, 2);
            this.tlpAdd.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tlpAdd.Location = new System.Drawing.Point(3, 17);
            this.tlpAdd.Name = "tlpAdd";
            this.tlpAdd.RowCount = 3;
            this.tlpAdd.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tlpAdd.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tlpAdd.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tlpAdd.Size = new System.Drawing.Size(373, 168);
            this.tlpAdd.TabIndex = 0;
            // 
            // radCustom
            // 
            this.radCustom.AutoSize = true;
            this.radCustom.Location = new System.Drawing.Point(3, 57);
            this.radCustom.Name = "radCustom";
            this.radCustom.Size = new System.Drawing.Size(65, 16);
            this.radCustom.TabIndex = 3;
            this.radCustom.Text = "Custom:";
            this.radCustom.UseVisualStyleBackColor = true;
            // 
            // txtProActionRocky
            // 
            this.txtProActionRocky.Location = new System.Drawing.Point(134, 30);
            this.txtProActionRocky.MaxLength = 8;
            this.txtProActionRocky.Name = "txtProActionRocky";
            this.txtProActionRocky.Size = new System.Drawing.Size(71, 21);
            this.txtProActionRocky.TabIndex = 1;
            this.txtProActionRocky.Enter += new System.EventHandler(this.txtProActionRocky_Enter);
            // 
            // txtGameGenie
            // 
            this.txtGameGenie.Location = new System.Drawing.Point(134, 3);
            this.txtGameGenie.MaxLength = 8;
            this.txtGameGenie.Name = "txtGameGenie";
            this.txtGameGenie.Size = new System.Drawing.Size(71, 21);
            this.txtGameGenie.TabIndex = 1;
            this.txtGameGenie.Enter += new System.EventHandler(this.txtGameGenie_Enter);
            // 
            // radGameGenie
            // 
            this.radGameGenie.AutoSize = true;
            this.radGameGenie.Checked = true;
            this.radGameGenie.Location = new System.Drawing.Point(3, 3);
            this.radGameGenie.Name = "radGameGenie";
            this.radGameGenie.Size = new System.Drawing.Size(89, 16);
            this.radGameGenie.TabIndex = 2;
            this.radGameGenie.TabStop = true;
            this.radGameGenie.Text = "Game Genie:";
            this.radGameGenie.UseVisualStyleBackColor = true;
            // 
            // radProActionRocky
            // 
            this.radProActionRocky.AutoSize = true;
            this.radProActionRocky.Location = new System.Drawing.Point(3, 30);
            this.radProActionRocky.Name = "radProActionRocky";
            this.radProActionRocky.Size = new System.Drawing.Size(125, 16);
            this.radProActionRocky.TabIndex = 2;
            this.radProActionRocky.Text = "Pro Action Rocky:";
            this.radProActionRocky.UseVisualStyleBackColor = true;
            // 
            // tlpCustom
            // 
            this.tlpCustom.ColumnCount = 2;
            this.tlpCustom.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tlpCustom.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tlpCustom.Controls.Add(this.lblAddress, 0, 0);
            this.tlpCustom.Controls.Add(this.txtAddress, 1, 0);
            this.tlpCustom.Controls.Add(this.lblNewValue, 0, 2);
            this.tlpCustom.Controls.Add(this.txtValue, 1, 2);
            this.tlpCustom.Controls.Add(this.txtCompare, 1, 3);
            this.tlpCustom.Controls.Add(this.chkCompareValue, 0, 3);
            this.tlpCustom.Controls.Add(this.tableLayoutPanel1, 0, 1);
            this.tlpCustom.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tlpCustom.Location = new System.Drawing.Point(134, 57);
            this.tlpCustom.Name = "tlpCustom";
            this.tlpCustom.RowCount = 6;
            this.tlpCustom.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tlpCustom.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tlpCustom.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tlpCustom.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tlpCustom.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tlpCustom.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tlpCustom.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
            this.tlpCustom.Size = new System.Drawing.Size(236, 108);
            this.tlpCustom.TabIndex = 4;
            // 
            // lblAddress
            // 
            this.lblAddress.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblAddress.AutoSize = true;
            this.lblAddress.Location = new System.Drawing.Point(3, 7);
            this.lblAddress.Name = "lblAddress";
            this.lblAddress.Size = new System.Drawing.Size(53, 12);
            this.lblAddress.TabIndex = 0;
            this.lblAddress.Text = "Address:";
            // 
            // txtAddress
            // 
            this.txtAddress.Location = new System.Drawing.Point(111, 3);
            this.txtAddress.MaxLength = 8;
            this.txtAddress.Name = "txtAddress";
            this.txtAddress.Size = new System.Drawing.Size(69, 21);
            this.txtAddress.TabIndex = 1;
            this.txtAddress.Enter += new System.EventHandler(this.customField_Enter);
            // 
            // lblNewValue
            // 
            this.lblNewValue.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblNewValue.AutoSize = true;
            this.lblNewValue.Location = new System.Drawing.Point(3, 58);
            this.lblNewValue.Name = "lblNewValue";
            this.lblNewValue.Size = new System.Drawing.Size(65, 12);
            this.lblNewValue.TabIndex = 3;
            this.lblNewValue.Text = "New Value:";
            // 
            // txtValue
            // 
            this.txtValue.Location = new System.Drawing.Point(111, 54);
            this.txtValue.MaxLength = 2;
            this.txtValue.Name = "txtValue";
            this.txtValue.Size = new System.Drawing.Size(30, 21);
            this.txtValue.TabIndex = 5;
            this.txtValue.Enter += new System.EventHandler(this.customField_Enter);
            // 
            // txtCompare
            // 
            this.txtCompare.Enabled = false;
            this.txtCompare.Location = new System.Drawing.Point(111, 81);
            this.txtCompare.MaxLength = 2;
            this.txtCompare.Name = "txtCompare";
            this.txtCompare.Size = new System.Drawing.Size(30, 21);
            this.txtCompare.TabIndex = 6;
            // 
            // chkCompareValue
            // 
            this.chkCompareValue.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.chkCompareValue.AutoSize = true;
            this.chkCompareValue.Location = new System.Drawing.Point(3, 83);
            this.chkCompareValue.Name = "chkCompareValue";
            this.chkCompareValue.Size = new System.Drawing.Size(102, 16);
            this.chkCompareValue.TabIndex = 7;
            this.chkCompareValue.Text = "Compare Value";
            this.chkCompareValue.UseVisualStyleBackColor = true;
            this.chkCompareValue.CheckedChanged += new System.EventHandler(this.chkCompareValue_CheckedChanged);
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 2;
            this.tlpCustom.SetColumnSpan(this.tableLayoutPanel1, 2);
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Controls.Add(this.radAbsoluteAddress, 1, 0);
            this.tableLayoutPanel1.Controls.Add(this.radRelativeAddress, 0, 0);
            this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 27);
            this.tableLayoutPanel1.Margin = new System.Windows.Forms.Padding(0);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 1;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(200, 24);
            this.tableLayoutPanel1.TabIndex = 8;
            // 
            // radAbsoluteAddress
            // 
            this.radAbsoluteAddress.AutoSize = true;
            this.radAbsoluteAddress.Location = new System.Drawing.Point(68, 3);
            this.radAbsoluteAddress.Name = "radAbsoluteAddress";
            this.radAbsoluteAddress.Size = new System.Drawing.Size(77, 16);
            this.radAbsoluteAddress.TabIndex = 1;
            this.radAbsoluteAddress.Text = "Game Code";
            this.radAbsoluteAddress.UseVisualStyleBackColor = true;
            this.radAbsoluteAddress.Enter += new System.EventHandler(this.customField_Enter);
            // 
            // radRelativeAddress
            // 
            this.radRelativeAddress.AutoSize = true;
            this.radRelativeAddress.Checked = true;
            this.radRelativeAddress.Location = new System.Drawing.Point(3, 3);
            this.radRelativeAddress.Name = "radRelativeAddress";
            this.radRelativeAddress.Size = new System.Drawing.Size(59, 16);
            this.radRelativeAddress.TabIndex = 0;
            this.radRelativeAddress.TabStop = true;
            this.radRelativeAddress.Text = "Memory";
            this.radRelativeAddress.UseVisualStyleBackColor = true;
            this.radRelativeAddress.Enter += new System.EventHandler(this.customField_Enter);
            // 
            // chkEnabled
            // 
            this.chkEnabled.AutoSize = true;
            this.tableLayoutPanel2.SetColumnSpan(this.chkEnabled, 2);
            this.chkEnabled.Location = new System.Drawing.Point(3, 59);
            this.chkEnabled.Name = "chkEnabled";
            this.chkEnabled.Size = new System.Drawing.Size(102, 16);
            this.chkEnabled.TabIndex = 6;
            this.chkEnabled.Text = "Cheat Enabled";
            this.chkEnabled.UseVisualStyleBackColor = true;
            // 
            // txtGameName
            // 
            this.txtGameName.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.txtGameName.Location = new System.Drawing.Point(80, 4);
            this.txtGameName.Name = "txtGameName";
            this.txtGameName.ReadOnly = true;
            this.txtGameName.Size = new System.Drawing.Size(227, 21);
            this.txtGameName.TabIndex = 0;
            // 
            // frmCheat
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.ClientSize = new System.Drawing.Size(385, 301);
            this.Controls.Add(this.tableLayoutPanel2);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(401, 340);
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(401, 340);
            this.Name = "frmCheat";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Edit Cheat";
            this.Controls.SetChildIndex(this.baseConfigPanel, 0);
            this.Controls.SetChildIndex(this.tableLayoutPanel2, 0);
            this.tableLayoutPanel2.ResumeLayout(false);
            this.tableLayoutPanel2.PerformLayout();
            this.grpCode.ResumeLayout(false);
            this.tlpAdd.ResumeLayout(false);
            this.tlpAdd.PerformLayout();
            this.tlpCustom.ResumeLayout(false);
            this.tlpCustom.PerformLayout();
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox txtCheatName;
		private System.Windows.Forms.GroupBox grpCode;
		private System.Windows.Forms.TableLayoutPanel tlpAdd;
		private System.Windows.Forms.RadioButton radCustom;
		private System.Windows.Forms.TextBox txtProActionRocky;
		private System.Windows.Forms.TextBox txtGameGenie;
		private System.Windows.Forms.RadioButton radGameGenie;
		private System.Windows.Forms.RadioButton radProActionRocky;
		private System.Windows.Forms.TableLayoutPanel tlpCustom;
		private System.Windows.Forms.Label lblAddress;
		private System.Windows.Forms.TextBox txtAddress;
		private System.Windows.Forms.Label lblNewValue;
		private System.Windows.Forms.RadioButton radRelativeAddress;
		private System.Windows.Forms.RadioButton radAbsoluteAddress;
		private System.Windows.Forms.TextBox txtValue;
		private System.Windows.Forms.TextBox txtCompare;
		private System.Windows.Forms.TextBox txtGameName;
		private System.Windows.Forms.Button btnBrowse;
		private System.Windows.Forms.CheckBox chkEnabled;
		private System.Windows.Forms.CheckBox chkCompareValue;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
	}
}