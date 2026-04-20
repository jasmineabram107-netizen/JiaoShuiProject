namespace TestCSharp
{
    partial class FrmMain
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
            if (disposing && (components != null))
            {
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
            this.btnBrowse = new System.Windows.Forms.Button();
            this.txtPath = new System.Windows.Forms.TextBox();
            this.picImage = new System.Windows.Forms.PictureBox();
            this.lblModule = new System.Windows.Forms.Label();
            this.btnTest = new System.Windows.Forms.Button();
            this.openDlg = new System.Windows.Forms.OpenFileDialog();
            this.label1 = new System.Windows.Forms.Label();
            this.cbDropletFit = new System.Windows.Forms.ComboBox();
            this.label2 = new System.Windows.Forms.Label();
            this.cbBaselineFit = new System.Windows.Forms.ComboBox();
            this.chkSubpixel = new System.Windows.Forms.CheckBox();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.chkRotate = new System.Windows.Forms.CheckBox();
            this.chkHasPin = new System.Windows.Forms.CheckBox();
            this.chkManual = new System.Windows.Forms.CheckBox();
            this.label3 = new System.Windows.Forms.Label();
            this.cbHorMode = new System.Windows.Forms.ComboBox();
            this.label4 = new System.Windows.Forms.Label();
            this.cbMainShape = new System.Windows.Forms.ComboBox();
            this.label5 = new System.Windows.Forms.Label();
            this.chkResString = new System.Windows.Forms.CheckBox();
            this.chkResAngle = new System.Windows.Forms.CheckBox();
            this.chkResBasept = new System.Windows.Forms.CheckBox();
            this.chkResContour = new System.Windows.Forms.CheckBox();
            this.btnRecog = new System.Windows.Forms.Button();
            this.chkSupplyBoundary = new System.Windows.Forms.CheckBox();
            this.label6 = new System.Windows.Forms.Label();
            this.cbFitMode = new System.Windows.Forms.ComboBox();
            this.btnGetbasePts = new System.Windows.Forms.Button();
            this.txtP1X = new System.Windows.Forms.TextBox();
            this.txtP1Y = new System.Windows.Forms.TextBox();
            this.txtP2Y = new System.Windows.Forms.TextBox();
            this.txtP2X = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.picImage)).BeginInit();
            this.SuspendLayout();
            // 
            // btnBrowse
            // 
            this.btnBrowse.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnBrowse.Location = new System.Drawing.Point(1015, 13);
            this.btnBrowse.Margin = new System.Windows.Forms.Padding(4);
            this.btnBrowse.Name = "btnBrowse";
            this.btnBrowse.Size = new System.Drawing.Size(39, 23);
            this.btnBrowse.TabIndex = 0;
            this.btnBrowse.Text = "...";
            this.btnBrowse.UseVisualStyleBackColor = true;
            this.btnBrowse.Click += new System.EventHandler(this.btnBrowse_Click);
            // 
            // txtPath
            // 
            this.txtPath.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtPath.Location = new System.Drawing.Point(105, 13);
            this.txtPath.Margin = new System.Windows.Forms.Padding(4);
            this.txtPath.Name = "txtPath";
            this.txtPath.Size = new System.Drawing.Size(902, 22);
            this.txtPath.TabIndex = 1;
            // 
            // picImage
            // 
            this.picImage.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.picImage.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.picImage.Location = new System.Drawing.Point(294, 46);
            this.picImage.Margin = new System.Windows.Forms.Padding(4);
            this.picImage.Name = "picImage";
            this.picImage.Size = new System.Drawing.Size(760, 557);
            this.picImage.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.picImage.TabIndex = 2;
            this.picImage.TabStop = false;
            this.picImage.Paint += new System.Windows.Forms.PaintEventHandler(this.picImage_Paint);
            this.picImage.MouseDown += new System.Windows.Forms.MouseEventHandler(this.picImage_MouseDown);
            this.picImage.MouseMove += new System.Windows.Forms.MouseEventHandler(this.picImage_MouseMove);
            this.picImage.MouseUp += new System.Windows.Forms.MouseEventHandler(this.picImage_MouseUp);
            this.picImage.Resize += new System.EventHandler(this.picImage_Resize);
            // 
            // lblModule
            // 
            this.lblModule.AutoSize = true;
            this.lblModule.Location = new System.Drawing.Point(9, 16);
            this.lblModule.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.lblModule.Name = "lblModule";
            this.lblModule.Size = new System.Drawing.Size(61, 13);
            this.lblModule.TabIndex = 3;
            this.lblModule.Text = "1. 径路:";
            // 
            // btnTest
            // 
            this.btnTest.Location = new System.Drawing.Point(381, 394);
            this.btnTest.Margin = new System.Windows.Forms.Padding(4);
            this.btnTest.Name = "btnTest";
            this.btnTest.Size = new System.Drawing.Size(94, 22);
            this.btnTest.TabIndex = 5;
            this.btnTest.Text = "识别(Old)";
            this.btnTest.UseVisualStyleBackColor = true;
            this.btnTest.Visible = false;
            // 
            // openDlg
            // 
            this.openDlg.FileName = "Open image file";
            this.openDlg.Filter = "Image Files|*.jpg;*.jpeg;*.png;*.bmp;*.gif|All files|*.*";
            this.openDlg.Title = "Select an Image";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(13, 311);
            this.label1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(72, 13);
            this.label1.TabIndex = 6;
            this.label1.Text = "液滴拟合：";
            // 
            // cbDropletFit
            // 
            this.cbDropletFit.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbDropletFit.FormattingEnabled = true;
            this.cbDropletFit.Items.AddRange(new object[] {
            "圆拟合",
            "CV椭圆拟合",
            "CV椭圆拟合（AMS)",
            "CV椭圆拟合（Direct)",
            "双椭圆拟合",
            "双多项式",
            "高宽法",
            "多项式（极坐）",
            "双圆拟合"});
            this.cbDropletFit.Location = new System.Drawing.Point(91, 311);
            this.cbDropletFit.Margin = new System.Windows.Forms.Padding(4);
            this.cbDropletFit.Name = "cbDropletFit";
            this.cbDropletFit.Size = new System.Drawing.Size(193, 21);
            this.cbDropletFit.TabIndex = 7;
            this.cbDropletFit.SelectedIndexChanged += new System.EventHandler(this.cbDropletFit_SelectedIndexChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(13, 340);
            this.label2.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(72, 13);
            this.label2.TabIndex = 8;
            this.label2.Text = "基线拟合：";
            // 
            // cbBaselineFit
            // 
            this.cbBaselineFit.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbBaselineFit.FormattingEnabled = true;
            this.cbBaselineFit.Items.AddRange(new object[] {
            "圆拟合",
            "CV椭圆拟合",
            "CV椭圆拟合（AMS)",
            "CV椭圆拟合（Direct)"});
            this.cbBaselineFit.Location = new System.Drawing.Point(91, 340);
            this.cbBaselineFit.Margin = new System.Windows.Forms.Padding(4);
            this.cbBaselineFit.Name = "cbBaselineFit";
            this.cbBaselineFit.Size = new System.Drawing.Size(193, 21);
            this.cbBaselineFit.TabIndex = 9;
            this.cbBaselineFit.SelectedIndexChanged += new System.EventHandler(this.cbBaselineFit_SelectedIndexChanged);
            // 
            // chkSubpixel
            // 
            this.chkSubpixel.AutoSize = true;
            this.chkSubpixel.Checked = true;
            this.chkSubpixel.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkSubpixel.Location = new System.Drawing.Point(105, 113);
            this.chkSubpixel.Margin = new System.Windows.Forms.Padding(4);
            this.chkSubpixel.Name = "chkSubpixel";
            this.chkSubpixel.Size = new System.Drawing.Size(91, 17);
            this.chkSubpixel.TabIndex = 10;
            this.chkSubpixel.Text = "亞像素边缘";
            this.chkSubpixel.UseVisualStyleBackColor = true;
            this.chkSubpixel.CheckedChanged += new System.EventHandler(this.chkSubpixel_CheckedChanged);
            // 
            // textBox1
            // 
            this.textBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.textBox1.Font = new System.Drawing.Font("Consolas", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBox1.Location = new System.Drawing.Point(14, 449);
            this.textBox1.Margin = new System.Windows.Forms.Padding(4);
            this.textBox1.Multiline = true;
            this.textBox1.Name = "textBox1";
            this.textBox1.ReadOnly = true;
            this.textBox1.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBox1.Size = new System.Drawing.Size(272, 155);
            this.textBox1.TabIndex = 11;
            this.textBox1.WordWrap = false;
            // 
            // chkRotate
            // 
            this.chkRotate.AutoSize = true;
            this.chkRotate.Location = new System.Drawing.Point(229, 136);
            this.chkRotate.Name = "chkRotate";
            this.chkRotate.Size = new System.Drawing.Size(52, 17);
            this.chkRotate.TabIndex = 12;
            this.chkRotate.Text = "倾斜";
            this.chkRotate.UseVisualStyleBackColor = true;
            this.chkRotate.CheckedChanged += new System.EventHandler(this.chkRotate_CheckedChanged);
            // 
            // chkHasPin
            // 
            this.chkHasPin.AutoSize = true;
            this.chkHasPin.Location = new System.Drawing.Point(144, 137);
            this.chkHasPin.Name = "chkHasPin";
            this.chkHasPin.Size = new System.Drawing.Size(52, 17);
            this.chkHasPin.TabIndex = 13;
            this.chkHasPin.Text = "插针";
            this.chkHasPin.UseVisualStyleBackColor = true;
            this.chkHasPin.CheckedChanged += new System.EventHandler(this.chkHasPin_CheckedChanged);
            // 
            // chkManual
            // 
            this.chkManual.AutoSize = true;
            this.chkManual.Location = new System.Drawing.Point(33, 137);
            this.chkManual.Name = "chkManual";
            this.chkManual.Size = new System.Drawing.Size(78, 17);
            this.chkManual.TabIndex = 15;
            this.chkManual.Text = "手动设置";
            this.chkManual.UseVisualStyleBackColor = true;
            this.chkManual.CheckedChanged += new System.EventHandler(this.chkManual_CheckedChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(30, 171);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(72, 13);
            this.label3.TabIndex = 16;
            this.label3.Text = "处理方式：";
            // 
            // cbHorMode
            // 
            this.cbHorMode.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbHorMode.FormattingEnabled = true;
            this.cbHorMode.Items.AddRange(new object[] {
            "未知",
            "凸透镜",
            "葫芦",
            "查看内部",
            "蘑菇菌盖"});
            this.cbHorMode.Location = new System.Drawing.Point(105, 167);
            this.cbHorMode.Name = "cbHorMode";
            this.cbHorMode.Size = new System.Drawing.Size(179, 21);
            this.cbHorMode.TabIndex = 17;
            this.cbHorMode.SelectedIndexChanged += new System.EventHandler(this.cbHorMode_SelectedIndexChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(30, 84);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(72, 13);
            this.label4.TabIndex = 18;
            this.label4.Text = "基线形状：";
            // 
            // cbMainShape
            // 
            this.cbMainShape.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbMainShape.FormattingEnabled = true;
            this.cbMainShape.Items.AddRange(new object[] {
            "未知",
            "凸面",
            "凹面",
            "水平面"});
            this.cbMainShape.Location = new System.Drawing.Point(105, 80);
            this.cbMainShape.Name = "cbMainShape";
            this.cbMainShape.Size = new System.Drawing.Size(179, 21);
            this.cbMainShape.TabIndex = 19;
            this.cbMainShape.SelectedIndexChanged += new System.EventHandler(this.cbMainShape_SelectedIndexChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(16, 432);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(42, 13);
            this.label5.TabIndex = 20;
            this.label5.Text = "JSON:";
            // 
            // chkResString
            // 
            this.chkResString.AutoSize = true;
            this.chkResString.Checked = true;
            this.chkResString.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkResString.Location = new System.Drawing.Point(16, 399);
            this.chkResString.Name = "chkResString";
            this.chkResString.Size = new System.Drawing.Size(91, 17);
            this.chkResString.TabIndex = 21;
            this.chkResString.Text = "结果字符串";
            this.chkResString.UseVisualStyleBackColor = true;
            this.chkResString.CheckedChanged += new System.EventHandler(this.chkResString_CheckedChanged);
            // 
            // chkResAngle
            // 
            this.chkResAngle.AutoSize = true;
            this.chkResAngle.Checked = true;
            this.chkResAngle.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkResAngle.Location = new System.Drawing.Point(113, 399);
            this.chkResAngle.Name = "chkResAngle";
            this.chkResAngle.Size = new System.Drawing.Size(52, 17);
            this.chkResAngle.TabIndex = 22;
            this.chkResAngle.Text = "角度";
            this.chkResAngle.UseVisualStyleBackColor = true;
            this.chkResAngle.CheckedChanged += new System.EventHandler(this.chkResAngle_CheckedChanged);
            // 
            // chkResBasept
            // 
            this.chkResBasept.AutoSize = true;
            this.chkResBasept.Checked = true;
            this.chkResBasept.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkResBasept.Location = new System.Drawing.Point(171, 399);
            this.chkResBasept.Name = "chkResBasept";
            this.chkResBasept.Size = new System.Drawing.Size(52, 17);
            this.chkResBasept.TabIndex = 23;
            this.chkResBasept.Text = "基点";
            this.chkResBasept.UseVisualStyleBackColor = true;
            this.chkResBasept.CheckedChanged += new System.EventHandler(this.chkResBasept_CheckedChanged);
            // 
            // chkResContour
            // 
            this.chkResContour.AutoSize = true;
            this.chkResContour.Checked = true;
            this.chkResContour.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkResContour.Location = new System.Drawing.Point(233, 399);
            this.chkResContour.Name = "chkResContour";
            this.chkResContour.Size = new System.Drawing.Size(52, 17);
            this.chkResContour.TabIndex = 24;
            this.chkResContour.Text = "拟合";
            this.chkResContour.UseVisualStyleBackColor = true;
            this.chkResContour.CheckedChanged += new System.EventHandler(this.chkResContour_CheckedChanged);
            // 
            // btnRecog
            // 
            this.btnRecog.Location = new System.Drawing.Point(191, 369);
            this.btnRecog.Margin = new System.Windows.Forms.Padding(4);
            this.btnRecog.Name = "btnRecog";
            this.btnRecog.Size = new System.Drawing.Size(94, 22);
            this.btnRecog.TabIndex = 5;
            this.btnRecog.Text = "识别";
            this.btnRecog.UseVisualStyleBackColor = true;
            this.btnRecog.Click += new System.EventHandler(this.btnRecog_Click);
            // 
            // chkSupplyBoundary
            // 
            this.chkSupplyBoundary.AutoSize = true;
            this.chkSupplyBoundary.Checked = true;
            this.chkSupplyBoundary.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkSupplyBoundary.Location = new System.Drawing.Point(206, 113);
            this.chkSupplyBoundary.Name = "chkSupplyBoundary";
            this.chkSupplyBoundary.Size = new System.Drawing.Size(78, 17);
            this.chkSupplyBoundary.TabIndex = 25;
            this.chkSupplyBoundary.Text = "边界补充";
            this.chkSupplyBoundary.UseVisualStyleBackColor = true;
            this.chkSupplyBoundary.CheckedChanged += new System.EventHandler(this.chkSupplyBoundary_CheckedChanged);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(9, 52);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(87, 13);
            this.label6.TabIndex = 26;
            this.label6.Text = "2. 拟合模式:";
            // 
            // cbFitMode
            // 
            this.cbFitMode.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbFitMode.FormattingEnabled = true;
            this.cbFitMode.Items.AddRange(new object[] {
            "自动拟合",
            "找基点",
            "找基线",
            "半自动(液滴自动，基线手动)",
            "手动拟合"});
            this.cbFitMode.Location = new System.Drawing.Point(105, 48);
            this.cbFitMode.Name = "cbFitMode";
            this.cbFitMode.Size = new System.Drawing.Size(180, 21);
            this.cbFitMode.TabIndex = 27;
            this.cbFitMode.SelectedIndexChanged += new System.EventHandler(this.cbFitMode_SelectedIndexChanged);
            // 
            // btnGetbasePts
            // 
            this.btnGetbasePts.Location = new System.Drawing.Point(12, 195);
            this.btnGetbasePts.Name = "btnGetbasePts";
            this.btnGetbasePts.Size = new System.Drawing.Size(134, 23);
            this.btnGetbasePts.TabIndex = 28;
            this.btnGetbasePts.Text = "3. 获取基线位置";
            this.btnGetbasePts.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.btnGetbasePts.UseVisualStyleBackColor = true;
            this.btnGetbasePts.Click += new System.EventHandler(this.btnGetbasePts_Click);
            // 
            // txtP1X
            // 
            this.txtP1X.Location = new System.Drawing.Point(113, 227);
            this.txtP1X.Name = "txtP1X";
            this.txtP1X.Size = new System.Drawing.Size(75, 22);
            this.txtP1X.TabIndex = 29;
            // 
            // txtP1Y
            // 
            this.txtP1Y.Location = new System.Drawing.Point(206, 227);
            this.txtP1Y.Name = "txtP1Y";
            this.txtP1Y.Size = new System.Drawing.Size(75, 22);
            this.txtP1Y.TabIndex = 30;
            // 
            // txtP2Y
            // 
            this.txtP2Y.Location = new System.Drawing.Point(206, 255);
            this.txtP2Y.Name = "txtP2Y";
            this.txtP2Y.Size = new System.Drawing.Size(75, 22);
            this.txtP2Y.TabIndex = 32;
            // 
            // txtP2X
            // 
            this.txtP2X.Location = new System.Drawing.Point(113, 255);
            this.txtP2X.Name = "txtP2X";
            this.txtP2X.Size = new System.Drawing.Size(75, 22);
            this.txtP2X.TabIndex = 31;
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(71, 232);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(33, 13);
            this.label7.TabIndex = 33;
            this.label7.Text = "左：";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(71, 259);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(33, 13);
            this.label8.TabIndex = 34;
            this.label8.Text = "右：";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(12, 284);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(54, 13);
            this.label9.TabIndex = 35;
            this.label9.Text = "4. 拟合";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(12, 377);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(54, 13);
            this.label10.TabIndex = 36;
            this.label10.Text = "5. 结果";
            // 
            // FrmMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1069, 616);
            this.Controls.Add(this.label10);
            this.Controls.Add(this.label9);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.txtP2Y);
            this.Controls.Add(this.txtP2X);
            this.Controls.Add(this.txtP1Y);
            this.Controls.Add(this.txtP1X);
            this.Controls.Add(this.btnGetbasePts);
            this.Controls.Add(this.cbFitMode);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.chkSupplyBoundary);
            this.Controls.Add(this.chkResContour);
            this.Controls.Add(this.chkResBasept);
            this.Controls.Add(this.chkResAngle);
            this.Controls.Add(this.chkResString);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.cbMainShape);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.cbHorMode);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.chkManual);
            this.Controls.Add(this.chkHasPin);
            this.Controls.Add(this.chkRotate);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.chkSubpixel);
            this.Controls.Add(this.cbBaselineFit);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.cbDropletFit);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.btnRecog);
            this.Controls.Add(this.btnTest);
            this.Controls.Add(this.lblModule);
            this.Controls.Add(this.picImage);
            this.Controls.Add(this.txtPath);
            this.Controls.Add(this.btnBrowse);
            this.Font = new System.Drawing.Font("SimSun", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "FrmMain";
            this.Text = "接触角拟合";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.FrmMain_FormClosing);
            this.Load += new System.EventHandler(this.FrmMain_Load);
            ((System.ComponentModel.ISupportInitialize)(this.picImage)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btnBrowse;
        private System.Windows.Forms.TextBox txtPath;
        private System.Windows.Forms.PictureBox picImage;
        private System.Windows.Forms.Label lblModule;
        private System.Windows.Forms.Button btnTest;
        private System.Windows.Forms.OpenFileDialog openDlg;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox cbDropletFit;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox cbBaselineFit;
        private System.Windows.Forms.CheckBox chkSubpixel;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.CheckBox chkRotate;
        private System.Windows.Forms.CheckBox chkHasPin;
        private System.Windows.Forms.CheckBox chkManual;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ComboBox cbHorMode;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.ComboBox cbMainShape;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.CheckBox chkResString;
        private System.Windows.Forms.CheckBox chkResAngle;
        private System.Windows.Forms.CheckBox chkResBasept;
        private System.Windows.Forms.CheckBox chkResContour;
        private System.Windows.Forms.Button btnRecog;
        private System.Windows.Forms.CheckBox chkSupplyBoundary;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.ComboBox cbFitMode;
        private System.Windows.Forms.Button btnGetbasePts;
        private System.Windows.Forms.TextBox txtP1X;
        private System.Windows.Forms.TextBox txtP1Y;
        private System.Windows.Forms.TextBox txtP2Y;
        private System.Windows.Forms.TextBox txtP2X;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label10;
    }
}

