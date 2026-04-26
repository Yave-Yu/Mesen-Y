using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Config;
using Mesen.GUI.Controls;

namespace Mesen.GUI.Forms.Config
{
	public partial class frmVideoConfig : BaseConfigForm
	{
		private Int32[] _paletteData;

		public frmVideoConfig()
		{
			InitializeComponent();

			ThemeHelper.FixMonoColors(contextPaletteList);
			ThemeHelper.FixMonoColors(contextPicturePresets);

			Entity = ConfigManager.Config.VideoInfo;

			AddBinding("ShowFPS", chkShowFps);
			AddBinding("UseBilinearInterpolation", chkBilinearInterpolation);
			AddBinding("VerticalSync", chkVerticalSync);
			AddBinding("UseHdPacks", chkUseHdPacks);
			AddBinding("IntegerFpsMode", chkIntegerFpsMode);
			AddBinding("FullscreenForceIntegerScale", chkFullscreenForceIntegerScale);
			AddBinding("UseExclusiveFullscreen", chkUseExclusiveFullscreen);
			AddBinding("ExclusiveFullscreenRefreshRate", cboRefreshRate);

			AddBinding("VideoScale", nudScale);
			AddBinding("AspectRatio", cboAspectRatio);
			AddBinding("CustomAspectRatio", nudCustomRatio);
			AddBinding("VideoFilter", cboFilter);

			AddBinding("ScreenRotation", cboScreenRotation);

			AddBinding("OverscanLeft", nudOverscanLeft);
			AddBinding("OverscanRight", nudOverscanRight);
			AddBinding("OverscanTop", nudOverscanTop);
			AddBinding("OverscanBottom", nudOverscanBottom);

			AddBinding("Brightness", trkBrightness);
			AddBinding("Contrast", trkContrast);
			AddBinding("Hue", trkHue);
			AddBinding("Saturation", trkSaturation);
			AddBinding("ScanlineIntensity", trkScanlines);

			AddBinding("NtscArtifacts", trkArtifacts);
			AddBinding("NtscBleed", trkBleed);
			AddBinding("NtscFringing", trkFringing);
			AddBinding("NtscGamma", trkGamma);
			AddBinding("NtscResolution", trkResolution);
			AddBinding("NtscSharpness", trkSharpness);
			AddBinding("NtscMergeFields", chkMergeFields);
			AddBinding("NtscVerticalBlend", chkVerticalBlend);

			AddBinding("NtscYFilterLength", trkYFilterLength);
			AddBinding("NtscIFilterLength", trkIFilterLength);
			AddBinding("NtscQFilterLength", trkQFilterLength);

			AddBinding("RemoveSpriteLimit", chkRemoveSpriteLimit);
			AddBinding("AdaptiveSpriteLimit", chkAdaptiveSpriteLimit);
			AddBinding("DisableBackground", chkDisableBackground);
			AddBinding("DisableSprites", chkDisableSprites);
			AddBinding("ForceBackgroundFirstColumn", chkForceBackgroundFirstColumn);
			AddBinding("ForceSpritesFirstColumn", chkForceSpritesFirstColumn);

			AddBinding("UseCustomVsPalette", chkUseCustomVsPalette);

			AddBinding("ShowColorIndexes", chkShowColorIndexes);

			_paletteData = InteropEmu.GetRgbPalette();
			if (!ConfigManager.Config.VideoInfo.IsFullColorPalette())
			{
				Array.Resize(ref _paletteData, 64);
			}

			RefreshPalette();

			toolTip.SetToolTip(picHdNesTooltip, ResourceHelper.GetMessage("HDNesTooltip"));

			cboFullscreenResolution.Items.Insert(0, ResourceHelper.GetMessage("DefaultResolution"));
			if (cboFullscreenResolution.Items.Contains(ConfigManager.Config.VideoInfo.FullscreenResolution))
			{
				cboFullscreenResolution.SelectedItem = ConfigManager.Config.VideoInfo.FullscreenResolution;
			}
			else
			{
				cboFullscreenResolution.SelectedIndex = 0;
			}

			btnSelectPreset.Image = BaseControl.DownArrow;
			btnSelectPalette.Image = BaseControl.DownArrow;

			UpdateOverscanImage(picOverscan, (int)nudOverscanTop.Value, (int)nudOverscanBottom.Value, (int)nudOverscanLeft.Value, (int)nudOverscanRight.Value);
			UpdateOverscanImage(picGameSpecificOverscan, (int)nudGameSpecificOverscanTop.Value, (int)nudGameSpecificOverscanBottom.Value, (int)nudGameSpecificOverscanLeft.Value, (int)nudGameSpecificOverscanRight.Value);

			ResourceHelper.ApplyResources(this, contextPaletteList);
			ResourceHelper.ApplyResources(this, contextPicturePresets);

			if (Program.IsMono)
			{
				//Not available in the linux build (for now)
				chkUseExclusiveFullscreen.Visible = false;
			}
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

			//Do this after we localize the form
			InitializeGameSpecificOverscanTab();
		}

		private void InitializeGameSpecificOverscanTab()
		{
			RomInfo romInfo = InteropEmu.GetRomInfo();
			if (romInfo.PrgCrc32 == 0)
			{
				chkEnableGameSpecificOverscan.Enabled = false;
			}
			else
			{
				chkEnableGameSpecificOverscan.Text += " (" + romInfo.GetRomName() + ")";

				GameSpecificInfo info = GameSpecificInfo.GetGameSpecificInfo();
				if (info != null)
				{
					chkEnableGameSpecificOverscan.Checked = info.OverrideOverscan;
					if (chkEnableGameSpecificOverscan.Checked)
					{
						tabOverscan.SelectedTab = tpgOverscanGameSpecific;
					}
					nudGameSpecificOverscanTop.Value = info.OverscanTop;
					nudGameSpecificOverscanBottom.Value = info.OverscanBottom;
					nudGameSpecificOverscanLeft.Value = info.OverscanLeft;
					nudGameSpecificOverscanRight.Value = info.OverscanRight;
				}
			}

			//Change name to share resources with global overscan tab
			lblGameSpecificOverscanBottom.Name = "lblBottom";
			lblGameSpecificOverscanLeft.Name = "lblLeft";
			lblGameSpecificOverscanRight.Name = "lblRight";
			lblGameSpecificOverscanTop.Name = "lblTop";
		}

		private void UpdatePalette()
		{
			byte[] result = new byte[_paletteData.Length * sizeof(int)];
			Buffer.BlockCopy(_paletteData, 0, result, 0, result.Length);
			((VideoInfo)Entity).PaletteData = Convert.ToBase64String(result);
		}

		protected override bool ValidateInput()
		{
			UpdateObject();
			UpdateCustomRatioVisibility();
			UpdatePalette();
			VideoFilterType filter = ((VideoInfo)Entity).VideoFilter;
			if (filter == VideoFilterType.NTSC)
			{
				tlpNtscFilter1.Visible = true;
				tlpNtscFilter2.Visible = false;
				chkMergeFields.Visible = true;
				grpNtscFilter.Visible = true;
			}
			else if (filter == VideoFilterType.BisqwitNtsc || filter == VideoFilterType.BisqwitNtscHalfRes || filter == VideoFilterType.BisqwitNtscQuarterRes)
			{
				tlpNtscFilter1.Visible = true;
				tlpNtscFilter2.Visible = true;
				chkMergeFields.Visible = false;
				grpNtscFilter.Visible = true;
			}
			else
			{
				grpNtscFilter.Visible = false;
			}

			VideoInfo.ApplyConfig();

			if (chkEnableGameSpecificOverscan.Checked)
			{
				InteropEmu.SetOverscanDimensions(
					(UInt32)nudGameSpecificOverscanLeft.Value,
					(UInt32)nudGameSpecificOverscanRight.Value,
					(UInt32)nudGameSpecificOverscanTop.Value,
					(UInt32)nudGameSpecificOverscanBottom.Value
				);
			}

			return true;
		}

		protected override void OnFormClosed(FormClosedEventArgs e)
		{
			if (DialogResult == DialogResult.OK)
			{
				UpdatePalette();

				if (cboFullscreenResolution.SelectedIndex > 0)
				{
					ConfigManager.Config.VideoInfo.FullscreenResolution = cboFullscreenResolution.SelectedItem.ToString();
				}
				else
				{
					ConfigManager.Config.VideoInfo.FullscreenResolution = "";
				}

				GameSpecificInfo.SetGameSpecificOverscan(
					chkEnableGameSpecificOverscan.Checked,
					(UInt32)nudGameSpecificOverscanTop.Value,
					(UInt32)nudGameSpecificOverscanBottom.Value,
					(UInt32)nudGameSpecificOverscanLeft.Value,
					(UInt32)nudGameSpecificOverscanRight.Value
				);
			}
			base.OnFormClosed(e);
			VideoInfo.ApplyConfig();
		}

		private void RefreshPalette()
		{
			this.ctrlPaletteDisplay.ShowColorIndexes = chkShowColorIndexes.Checked;
			this.ctrlPaletteDisplay.PaletteData = _paletteData;
		}

		private void ctrlPaletteDisplay_ColorClick(int colorIndex)
		{
			colorDialog.SolidColorOnly = true;
			colorDialog.AllowFullOpen = true;
			colorDialog.FullOpen = true;
			colorDialog.Color = Color.FromArgb(_paletteData[colorIndex]);
			if (colorDialog.ShowDialog() == DialogResult.OK)
			{
				_paletteData[colorIndex] = colorDialog.Color.ToArgb();
			}

			RefreshPalette();
		}

		private void btnLoadPalFile_Click(object sender, EventArgs e)
		{
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.SetFilter("Palette Files (*.pal)|*.pal|All Files (*.*)|*.*");
			if (ofd.ShowDialog() == DialogResult.OK)
			{
				using (FileStream paletteFile = File.OpenRead(ofd.FileName))
				{
					byte[] paletteFileData = new byte[512 * 3];
					int byteCount = paletteFile.Read(paletteFileData, 0, 512 * 3);
					if (byteCount == 64 * 3 || byteCount == 512 * 3)
					{
						_paletteData = new Int32[byteCount / 3];
						for (int i = 0; i < byteCount; i += 3)
						{
							_paletteData[i / 3] = (Int32)((UInt32)0xFF000000 | (UInt32)paletteFileData[i + 2] | (UInt32)(paletteFileData[i + 1] << 8) | (UInt32)(paletteFileData[i] << 16));
						}
						RefreshPalette();

						byte[] bytePalette = new byte[_paletteData.Length * sizeof(int)];
						Buffer.BlockCopy(_paletteData, 0, bytePalette, 0, bytePalette.Length);
						ConfigManager.Config.VideoInfo.AddPalette(Path.GetFileNameWithoutExtension(ofd.FileName), bytePalette);
					}
					paletteFile.Close();
				}
			}
		}

		private void UpdateOverscanImage(PictureBox picture, int top, int bottom, int left, int right)
		{
			Bitmap overscan = new Bitmap(picture.Width - 2, picture.Height - 2, PixelFormat.Format32bppPArgb);

			using (Graphics g = Graphics.FromImage(overscan))
			{
				g.Clear(Color.DarkGray);

				Rectangle fg = new Rectangle(left, top, 256 - left - right, 240 - top - bottom);
				g.ScaleTransform((float)overscan.Width / 256, (float)overscan.Height / 240);
				g.FillRectangle(Brushes.LightCyan, fg);
			}
			picture.Image = overscan;
		}

		private void nudOverscan_ValueChanged(object sender, EventArgs e)
		{
			UpdateOverscanImage(picOverscan, (int)nudOverscanTop.Value, (int)nudOverscanBottom.Value, (int)nudOverscanLeft.Value, (int)nudOverscanRight.Value);
		}

		private void nudGameSpecificOverscan_ValueChanged(object sender, EventArgs e)
		{
			UpdateOverscanImage(picGameSpecificOverscan, (int)nudGameSpecificOverscanTop.Value, (int)nudGameSpecificOverscanBottom.Value, (int)nudGameSpecificOverscanLeft.Value, (int)nudGameSpecificOverscanRight.Value);
		}

		private void btnResetPictureSettings_Click(object sender, EventArgs e)
		{
			cboFilter.SelectedIndex = 0;
			trkBrightness.Value = 0;
			trkContrast.Value = 0;
			trkHue.Value = 0;
			trkSaturation.Value = 0;

			trkScanlines.Value = 0;

			trkArtifacts.Value = 0;
			trkBleed.Value = 0;
			trkFringing.Value = 0;
			trkGamma.Value = 0;
			trkResolution.Value = 0;
			trkSharpness.Value = 0;
			chkMergeFields.Checked = false;
			chkVerticalBlend.Checked = true;

			trkYFilterLength.Value = 0;
			trkIFilterLength.Value = 50;
			trkQFilterLength.Value = 50;
		}

		private void btnSelectPreset_Click(object sender, EventArgs e)
		{
			contextPicturePresets.Show(btnSelectPreset.PointToScreen(new Point(0, btnSelectPreset.Height - 1)));
		}

		private void mnuPresetComposite_Click(object sender, EventArgs e)
		{
			cboFilter.SelectedIndex = 1;
			trkHue.Value = 0;
			trkSaturation.Value = 0;
			trkContrast.Value = 0;
			trkBrightness.Value = 0;
			trkSharpness.Value = 0;
			trkGamma.Value = 0;
			trkResolution.Value = 0;
			trkArtifacts.Value = 0;
			trkFringing.Value = 0;
			trkBleed.Value = 0;
			chkMergeFields.Checked = false;

			trkScanlines.Value = 15;
		}

		private void mnuPresetSVideo_Click(object sender, EventArgs e)
		{
			cboFilter.SelectedIndex = 1;
			trkHue.Value = 0;
			trkSaturation.Value = 0;
			trkContrast.Value = 0;
			trkBrightness.Value = 0;
			trkSharpness.Value = 20;
			trkGamma.Value = 0;
			trkResolution.Value = 20;
			trkArtifacts.Value = -100;
			trkFringing.Value = -100;
			trkBleed.Value = 0;
			chkMergeFields.Checked = false;

			trkScanlines.Value = 15;
		}

		private void mnuPresetRgb_Click(object sender, EventArgs e)
		{
			cboFilter.SelectedIndex = 1;
			trkHue.Value = 0;
			trkSaturation.Value = 0;
			trkContrast.Value = 0;
			trkBrightness.Value = 0;
			trkSharpness.Value = 20;
			trkGamma.Value = 0;
			trkResolution.Value = 70;
			trkArtifacts.Value = -100;
			trkFringing.Value = -100;
			trkBleed.Value = -100;
			chkMergeFields.Checked = false;

			trkScanlines.Value = 15;
		}

		private void mnuPresetMonochrome_Click(object sender, EventArgs e)
		{
			cboFilter.SelectedIndex = 1;
			trkHue.Value = 0;
			trkSaturation.Value = -100;
			trkContrast.Value = 0;
			trkBrightness.Value = 0;
			trkSharpness.Value = 20;
			trkGamma.Value = 0;
			trkResolution.Value = 70;
			trkArtifacts.Value = -20;
			trkFringing.Value = -20;
			trkBleed.Value = -10;
			chkMergeFields.Checked = false;

			trkScanlines.Value = 15;
		}

		private void btnSelectPalette_Click(object sender, EventArgs e)
		{
			contextPaletteList.Show(btnSelectPalette.PointToScreen(new Point(0, btnSelectPalette.Height - 1)));
		}

		private void UpdatePalette(UInt32[] newPalette)
		{
			_paletteData = new Int32[newPalette.Length];
			for (int i = 0; i < newPalette.Length; i++)
			{
				_paletteData[i] = (Int32)newPalette[i];
			}
			RefreshPalette();
		}

		private void mnuDefaultPalette_Click(object sender, EventArgs e)
		{
			UpdatePalette(new UInt32[] { 0xFF6E6E6E, 0xFF001E96, 0xFF2800A0, 0xFF41008C, 0xFF5A005F, 0xFF64001E, 0xFF5F0F00, 0xFF501E00, 0xFF2D3700, 0xFF0F4600, 0xFF004B00, 0xFF004623, 0xFF00375F, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB4B4B4, 0xFF005AE6, 0xFF4637F0, 0xFF781EEB, 0xFFA500B4, 0xFFBE005F, 0xFFB92800, 0xFF965000, 0xFF6E7300, 0xFF2D8700, 0xFF008C00, 0xFF008746, 0xFF00789B, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF5AAFFF, 0xFF8796FF, 0xFFBE82FF, 0xFFEB6EFF, 0xFFFF6EC8, 0xFFFF8269, 0xFFEBA500, 0xFFC8C300, 0xFF82DC00, 0xFF3CE61E, 0xFF2DE182, 0xFF32D2DC, 0xFF505050, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFBEE6FF, 0xFFD2D7FF, 0xFFE6CDFF, 0xFFF5C8FF, 0xFFFFC8EB, 0xFFFFD2C3, 0xFFF5DCAF, 0xFFEBE6A0, 0xFFC8F0A0, 0xFFAFF5AA, 0xFFAAF0C8, 0xFFA0EBF0, 0xFFBEBEBE, 0xFF000000, 0xFF000000 });
		}

		private void mnuPaletteCompositeDirect_Click(object sender, EventArgs e)
		{
			UpdatePalette(new UInt32[] { 0xFF656565, 0xFF00127D, 0xFF18008E, 0xFF360082, 0xFF56005D, 0xFF5A0018, 0xFF4F0500, 0xFF381900, 0xFF1D3100, 0xFF043D00, 0xFF004100, 0xFF003B17, 0xFF002E55, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFAFAFAF, 0xFF194EC8, 0xFF472FE3, 0xFF6B1FD7, 0xFF931BAE, 0xFF9E1A5E, 0xFF993200, 0xFF7B4B00, 0xFF5B6700, 0xFF267A00, 0xFF008200, 0xFF007A3E, 0xFF006E8A, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF64A9FF, 0xFF8E89FF, 0xFFB676FF, 0xFFE06FFF, 0xFFEF6CC4, 0xFFF0806A, 0xFFD8982C, 0xFFB9B40A, 0xFF83CB0C, 0xFF5BD63F, 0xFF4AD17E, 0xFF4DC7CB, 0xFF4C4C4C, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFC7E5FF, 0xFFD9D9FF, 0xFFE9D1FF, 0xFFF9CEFF, 0xFFFFCCF1, 0xFFFFD4CB, 0xFFF8DFB1, 0xFFEDEAA4, 0xFFD6F4A4, 0xFFC5F8B8, 0xFFBEF6D3, 0xFFBFF1F1, 0xFFB9B9B9, 0xFF000000, 0xFF000000 });
		}

		private void mnuPaletteUnsaturated_Click(object sender, EventArgs e)
		{
			UpdatePalette(new UInt32[] { 0xFF6B6B6B, 0xFF001E87, 0xFF1F0B96, 0xFF3B0C87, 0xFF590D61, 0xFF5E0528, 0xFF551100, 0xFF461B00, 0xFF303200, 0xFF0A4800, 0xFF004E00, 0xFF004619, 0xFF003A58, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB2B2B2, 0xFF1A53D1, 0xFF4835EE, 0xFF7123EC, 0xFF9A1EB7, 0xFFA51E62, 0xFFA52D19, 0xFF874B00, 0xFF676900, 0xFF298400, 0xFF038B00, 0xFF008240, 0xFF007891, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF63ADFD, 0xFF908AFE, 0xFFB977FC, 0xFFE771FE, 0xFFF76FC9, 0xFFF5836A, 0xFFDD9C29, 0xFFBDB807, 0xFF84D107, 0xFF5BDC3B, 0xFF48D77D, 0xFF48CCCE, 0xFF555555, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFC4E3FE, 0xFFD7D5FE, 0xFFE6CDFE, 0xFFF9CAFE, 0xFFFEC9F0, 0xFFFED1C7, 0xFFF7DCAC, 0xFFE8E89C, 0xFFD1F29D, 0xFFBFF4B1, 0xFFB7F5CD, 0xFFB7F0EE, 0xFFBEBEBE, 0xFF000000, 0xFF000000 });
		}

		private void mnuPaletteSmooth_Click(object sender, EventArgs e)
		{
			UpdatePalette(new UInt32[] { 0xFF6A6A6A, 0xFF00148F, 0xFF1E029B, 0xFF3F008A, 0xFF600060, 0xFF660017, 0xFF570D00, 0xFF3C1F00, 0xFF1B3300, 0xFF034200, 0xFF004500, 0xFF003C1F, 0xFF00315C, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB4B4B4, 0xFF0F4BD4, 0xFF412DEB, 0xFF6C1DD9, 0xFF9C17AB, 0xFFA71A4D, 0xFF993200, 0xFF7C4A00, 0xFF546400, 0xFF1A7800, 0xFF007F00, 0xFF00763E, 0xFF00678F, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF68A6FF, 0xFF8C9CFF, 0xFFB586FF, 0xFFD975FD, 0xFFE377B9, 0xFFE58D68, 0xFFD49D29, 0xFFB3AF0C, 0xFF7BC211, 0xFF55CA47, 0xFF46CB81, 0xFF47C1C5, 0xFF4A4A4A, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFCCEAFF, 0xFFDCDEFF, 0xFFECDAFF, 0xFFF8D7FE, 0xFFFCD6F5, 0xFFFDDBCF, 0xFFF9E7B5, 0xFFF1F0AA, 0xFFDAFAA9, 0xFFC9FFBC, 0xFFC3FBD7, 0xFFC4F6F6, 0xFFBEBEBE, 0xFF000000, 0xFF000000 });
		}

		private void mnuPaletteDigitalPrime_Click(object sender, EventArgs e)
		{
			UpdatePalette(new UInt32[] { 0xFF696969, 0xFF00148F, 0xFF1E029B, 0xFF3F008A, 0xFF600060, 0xFF660017, 0xFF570D00, 0xFF451B00, 0xFF243400, 0xFF034200, 0xFF004500, 0xFF003C1F, 0xFF00315C, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFAFAFAF, 0xFF0F51DD, 0xFF442FF3, 0xFF7220E2, 0xFFA319B3, 0xFFAE1C51, 0xFFA43400, 0xFF884D00, 0xFF676D00, 0xFF208000, 0xFF008B00, 0xFF007F42, 0xFF006C97, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF65AAFF, 0xFF8C96FF, 0xFFB983FF, 0xFFDD6FFF, 0xFFEA6FBD, 0xFFEB8466, 0xFFDCA21F, 0xFFBAB403, 0xFF7ECB07, 0xFF54D33E, 0xFF3CD284, 0xFF3EC7CC, 0xFF4B4B4B, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFBDE2FF, 0xFFCECFFF, 0xFFE6C2FF, 0xFFF6BCFF, 0xFFF9C2ED, 0xFFFACFC6, 0xFFF8DEAC, 0xFFEEE9A1, 0xFFD0F59F, 0xFFBBF5AF, 0xFFB3F5CD, 0xFFB9EDF0, 0xFFB9B9B9, 0xFF000000, 0xFF000000 });
		}

		private void mnuPaletteWavebeam_Click(object sender, EventArgs e)
		{
			UpdatePalette(new UInt32[] { 0xFF6B6B6B, 0xFF001B87, 0xFF21009A, 0xFF40008C, 0xFF600067, 0xFF64001E, 0xFF590800, 0xFF461600, 0xFF263600, 0xFF024500, 0xFF004700, 0xFF00421D, 0xFF003659, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB4B4B4, 0xFF1555CE, 0xFF4337EA, 0xFF7124DA, 0xFF9C1AB6, 0xFFAA1164, 0xFFA82E00, 0xFF874B00, 0xFF666B00, 0xFF218300, 0xFF008A00, 0xFF008144, 0xFF007691, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF63AFFF, 0xFF8296FF, 0xFFC07DFE, 0xFFE977FF, 0xFFF572CD, 0xFFF4886B, 0xFFDDA029, 0xFFBDBD0A, 0xFF89D20E, 0xFF5CDE3E, 0xFF4BD886, 0xFF4DCFD2, 0xFF505050, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFBEE1FF, 0xFFD2D4FF, 0xFFE3CAFF, 0xFFF0C9FF, 0xFFFFC6E3, 0xFFFFCEC9, 0xFFF4DCAF, 0xFFEBE5A1, 0xFFD2EFA2, 0xFFBEF4B5, 0xFFB8F1D0, 0xFFB8EDF1, 0xFFBDBDBD, 0xFF000000, 0xFF000000 });
		}

		private void mnuPaletteNESClassic_Click(object sender, EventArgs e)
		{
			UpdatePalette(new UInt32[] { 0xFF606060, 0xFF000088, 0xFF200C98, 0xFF381478, 0xFF541460, 0xFF5C0010, 0xFF541000, 0xFF3C2408, 0xFF20340C, 0xFF0C400C, 0xFF184418, 0xFF003C20, 0xFF003058, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFA8A8A8, 0xFF0C4CC4, 0xFF4C24E0, 0xFF6814D0, 0xFF9014AC, 0xFF9C1C48, 0xFF903400, 0xFF745004, 0xFF5C6814, 0xFF187C10, 0xFF148008, 0xFF107448, 0xFF1C6490, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFCFCFC, 0xFF6498FC, 0xFF887CFC, 0xFFB068FC, 0xFFDC6CF4, 0xFFE870AC, 0xFFE48858, 0xFFCC9C20, 0xFFA8B000, 0xFF74C000, 0xFF5CCC50, 0xFF34C090, 0xFF50C0CC, 0xFF404040, 0xFF000000, 0xFF000000, 0xFFFCFCFC, 0xFFBCD4FC, 0xFFCCCCFC, 0xFFD8C4FC, 0xFFECC0FC, 0xFFF8C4E8, 0xFFF8CCC4, 0xFFE4CCA8, 0xFFD8DC9C, 0xFFC8E4A0, 0xFFC0E4B8, 0xFFB4ECC8, 0xFFB8E4EC, 0xFFBABABA, 0xFF000000, 0xFF000000 });
		}

		private void mnuPaletteNestopia_Click(object sender, EventArgs e)
		{
			UpdatePalette(new UInt32[] { 0xFF666666, 0xFF002A88, 0xFF1412A7, 0xFF3B00A4, 0xFF5C007E, 0xFF6E0040, 0xFF6C0700, 0xFF561D00, 0xFF333500, 0xFF0C4800, 0xFF005200, 0xFF004F08, 0xFF00404D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFADADAD, 0xFF155FD9, 0xFF4240FF, 0xFF7527FE, 0xFFA01ACC, 0xFFB71E7B, 0xFFB53120, 0xFF994E00, 0xFF6B6D00, 0xFF388700, 0xFF0D9300, 0xFF008F32, 0xFF007C8D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF64B0FF, 0xFF9290FF, 0xFFC676FF, 0xFFF26AFF, 0xFFFF6ECC, 0xFFFF8170, 0xFFEA9E22, 0xFFBCBE00, 0xFF88D800, 0xFF5CE430, 0xFF45E082, 0xFF48CDDE, 0xFF4F4F4F, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFC0DFFF, 0xFFD3D2FF, 0xFFE8C8FF, 0xFFFAC2FF, 0xFFFFC4EA, 0xFFFFCCC5, 0xFFF7D8A5, 0xFFE4E594, 0xFFCFEF96, 0xFFBDF4AB, 0xFFB3F3CC, 0xFFB5EBF2, 0xFFB8B8B8, 0xFF000000, 0xFF000000 });
		}

		private void mnuPaletteRockNES_Click(object sender, EventArgs e)
		{
			UpdatePalette(new UInt32[] { 0xFF5A5A5A, 0xFF001D66, 0xFF0F0E81, 0xFF29047E, 0xFF42005D, 0xFF50012C, 0xFF4E0801, 0xFF3D1500, 0xFF232400, 0xFF0A3200, 0xFF003900, 0xFF003708, 0xFF002D36, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFAEAEAE, 0xFF1951C0, 0xFF3A39E9, 0xFF6426E4, 0xFF8A1DB3, 0xFF9F2069, 0xFF9C2D21, 0xFF824300, 0xFF5B5D00, 0xFF327200, 0xFF147D00, 0xFF067A2D, 0xFF086A79, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF68ADFF, 0xFF928FFF, 0xFFC279FF, 0xFFED6EFF, 0xFFFF71C8, 0xFFFF8272, 0xFFE49C32, 0xFFB7BA14, 0xFF87D218, 0xFF61DE3C, 0xFF4EDB82, 0xFF51C8D9, 0xFF414141, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFCAE9FF, 0xFFDDDCFF, 0xFFF2D2FF, 0xFFFFCDFF, 0xFFFFCEF4, 0xFFFFD6CF, 0xFFFFE1B0, 0xFFEDEE9F, 0xFFD8F9A1, 0xFFC7FEB5, 0xFFBEFCD6, 0xFFBFF5FC, 0xFFB7B7B7, 0xFF000000, 0xFF000000 });
		}

		private void mnuPaletteNintendulator_Click(object sender, EventArgs e)
		{
			UpdatePalette(new UInt32[] { 0xFF626262, 0xFF002B96, 0xFF1210BA, 0xFF3E00B6, 0xFF63008A, 0xFF770043, 0xFF750300, 0xFF5C1C00, 0xFF353700, 0xFF084C00, 0xFF005700, 0xFF005403, 0xFF004453, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFABABAB, 0xFF0860F1, 0xFF3E3BFF, 0xFF7A1DFF, 0xFFAD0FE2, 0xFFC81381, 0xFFC42914, 0xFFA34B00, 0xFF6D7000, 0xFF308D00, 0xFF009C00, 0xFF00982A, 0xFF008296, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF56B1FF, 0xFF8E8BFF, 0xFFCD6CFF, 0xFFFF5DFF, 0xFFFF62D4, 0xFFFF7863, 0xFFF79C05, 0xFFBFC200, 0xFF80E100, 0xFF4CF015, 0xFF30EB79, 0xFF33D4EA, 0xFF4D4D4D, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFBBE0FF, 0xFFD2D1FF, 0xFFEBC4FF, 0xFFFFBEFF, 0xFFFFC0EE, 0xFFFFC9C1, 0xFFFCD79B, 0xFFE6E786, 0xFFCCF389, 0xFFB7F9A1, 0xFFACF7CA, 0xFFAEEEF7, 0xFFB8B8B8, 0xFF000000, 0xFF000000 });
		}

		private void mnuPaletteSonyCXA_Click(object sender, EventArgs e)
		{
			UpdatePalette(new UInt32[] { 0xFF585858, 0xFF00238C, 0xFF00139B, 0xFF2D0585, 0xFF5D0052, 0xFF7A0017, 0xFF7A0800, 0xFF5F1800, 0xFF352A00, 0xFF093900, 0xFF003F00, 0xFF003C22, 0xFF00325D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFA1A1A1, 0xFF0053EE, 0xFF153CFE, 0xFF6028E4, 0xFFA91D98, 0xFFD41E41, 0xFFD22C00, 0xFFAA4400, 0xFF6C5E00, 0xFF2D7300, 0xFF007D06, 0xFF007852, 0xFF0069A9, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF1FA5FE, 0xFF5E89FE, 0xFFB572FE, 0xFFFE65F6, 0xFFFE6790, 0xFFFE773C, 0xFFFE9308, 0xFFC4B200, 0xFF79CA10, 0xFF3AD54A, 0xFF11D1A4, 0xFF06BFFE, 0xFF424242, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFA0D9FE, 0xFFBDCCFE, 0xFFE1C2FE, 0xFFFEBCFB, 0xFFFEBDD0, 0xFFFEC5A9, 0xFFFED18E, 0xFFE9DE86, 0xFFC7E992, 0xFFA8EEB0, 0xFF95ECD9, 0xFF91E4FE, 0xFFACACAC, 0xFF000000, 0xFF000000 });
		}

		private void mnuPaletteZeroDegree_Click(object sender, EventArgs e)
		{
			UpdatePalette(new UInt32[] { 0xFF626262, 0xFF002C7D, 0xFF11169E, 0xFF36039E, 0xFF56007D, 0xFF680044, 0xFF680602, 0xFF561A00, 0xFF363100, 0xFF114300, 0xFF004E00, 0xFF004C02, 0xFF004044, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFAAAAAA, 0xFF1061D0, 0xFF3C43FC, 0xFF6E29FC, 0xFF9A1BD0, 0xFFB31D82, 0xFFB32D28, 0xFF9A4900, 0xFF6E6700, 0xFF3C8100, 0xFF108F00, 0xFF008D28, 0xFF007C82, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF5EB2FF, 0xFF8C93FF, 0xFFC078FF, 0xFFED6AFF, 0xFFFF6CD4, 0xFFFF7D77, 0xFFED9927, 0xFFC0B900, 0xFF8CD400, 0xFF5EE227, 0xFF44E077, 0xFF44CFD4, 0xFF4D4D4D, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFBEE0FF, 0xFFD0D3FF, 0xFFE5C9FF, 0xFFF8C3FF, 0xFFFFC3EE, 0xFFFFCAC8, 0xFFF8D6A8, 0xFFE5E395, 0xFFD0ED95, 0xFFBEF3A8, 0xFFB4F2C8, 0xFFB4EBEE, 0xFFB7B7B7, 0xFF000000, 0xFF000000 });
		}

		private void contextPaletteList_Opening(object sender, CancelEventArgs e)
		{
			for (int i = contextPaletteList.Items.Count - 1; i >= 13; i--)
			{
				contextPaletteList.Items.RemoveAt(i);
			}

			if (ConfigManager.Config.VideoInfo.SavedPalettes.Count > 0)
			{
				contextPaletteList.Items.Add(new ToolStripSeparator());
				foreach (PaletteInfo info in ConfigManager.Config.VideoInfo.SavedPalettes)
				{
					ToolStripItem item = contextPaletteList.Items.Add(info.Name);
					item.Click += (object o, EventArgs args) =>
					{
						_paletteData = ConfigManager.Config.VideoInfo.GetPalette(((ToolStripItem)o).Text);
						RefreshPalette();
					};
				}
			}
		}

		private void btnExportPalette_Click(object sender, EventArgs e)
		{
			SaveFileDialog sfd = new SaveFileDialog();
			sfd.SetFilter("Palette Files (*.pal)|*.pal");
			if (sfd.ShowDialog() == DialogResult.OK)
			{
				List<byte> bytePalette = new List<byte>();
				foreach (int value in _paletteData)
				{
					bytePalette.Add((byte)(value >> 16 & 0xFF));
					bytePalette.Add((byte)(value >> 8 & 0xFF));
					bytePalette.Add((byte)(value & 0xFF));
				}
				File.WriteAllBytes(sfd.FileName, bytePalette.ToArray());
			}
		}

		private void cboAspectRatio_SelectedIndexChanged(object sender, EventArgs e)
		{
			UpdateCustomRatioVisibility();
		}

		private void UpdateCustomRatioVisibility()
		{
			VideoAspectRatio ratio = cboAspectRatio.GetEnumValue<VideoAspectRatio>();
			lblCustomRatio.Visible = ratio == VideoAspectRatio.Custom;
			nudCustomRatio.Visible = ratio == VideoAspectRatio.Custom;
		}

		private void chkShowColorIndexes_CheckedChanged(object sender, EventArgs e)
		{
			this.RefreshPalette();
		}

		private void chkUseExclusiveFullscreen_CheckedChanged(object sender, EventArgs e)
		{
			flpRefreshRate.Visible = chkUseExclusiveFullscreen.Checked;
			flpResolution.Visible = chkUseExclusiveFullscreen.Checked;
		}

		private void chkEnableGameSpecificOverscan_CheckedChanged(object sender, EventArgs e)
		{
			tpgOverscanGameSpecific.ImageIndex = chkEnableGameSpecificOverscan.Checked ? 0 : -1;
			nudGameSpecificOverscanBottom.Enabled = chkEnableGameSpecificOverscan.Checked;
			nudGameSpecificOverscanLeft.Enabled = chkEnableGameSpecificOverscan.Checked;
			nudGameSpecificOverscanRight.Enabled = chkEnableGameSpecificOverscan.Checked;
			nudGameSpecificOverscanTop.Enabled = chkEnableGameSpecificOverscan.Checked;
		}

		private void chkRemoveSpriteLimit_CheckedChanged(object sender, EventArgs e)
		{
			chkAdaptiveSpriteLimit.Enabled = chkRemoveSpriteLimit.Checked;
		}
   }
}