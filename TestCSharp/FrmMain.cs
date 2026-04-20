using DropletLibCSharp;
using System;
using System.Collections.Generic;
using System.Diagnostics.Eventing.Reader;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;
using System.Windows.Forms;

namespace TestCSharp
{
    public partial class FrmMain : Form
    {
        // result variables
        private double[] m_angle;
        private PointAndAngle[] m_pointAndAngles;
        private EllipseFitBox[] m_boxs;
        private DropletLibCSharp.PointF[] m_leftFitRes;
        private DropletLibCSharp.PointF[] m_rightFitRes;
        private FittingGraphicsType m_fgType;

        private MainShapeType m_shape;

        private bool m_hasPin;
        private HorizonMode m_horMode;

        private DropletLibCSharp.RectangleF m_basePts;         
        private DropletLibCSharp.PointF[] m_keyPts;

        //private Point[] m_inBasePts = new Point[2];
        //private Point[] m_inBaselinePts = new Point[5];
        //private Point[] m_inDropletPts = new Point[5];

        //private List<Point> m_inBasePts = new List<Point>();
        private List<Point> m_inBaselinePts = new List<Point>();
        private List<Point> m_inDropletPts = new List<Point>();
        private bool draggingLeft = true; // true = leftPoints, false = rightPoints
        private int draggingIndex = -1;
        private int pointRadius = 3;

        //private int m_iCurPtMode = -1;  // 0: base points, 1: baseline, 2: droplet points

        //private int m_iPtIndex = -1;    
        //private bool m_bPtMove = false;
        private bool m_bDesignTime = true;

        double m_scaleX = 0;
        double m_scaleY = 0;

        public FrmMain()
        {
            InitializeComponent();
        }

        static string getAppPath()
        {
            string appPath = AppContext.BaseDirectory;
            return appPath;
        }

        private void init()
        {
            m_boxs = null;
            m_angle = null;
            m_leftFitRes = null;
            m_rightFitRes = null;
            m_fgType = FittingGraphicsType.eFgNone;

            m_pointAndAngles = new PointAndAngle[0];
            m_keyPts = new DropletLibCSharp.PointF[3];
        }

        private void FrmMain_Load(object sender, EventArgs e)
        {
            var appPath = getAppPath();
            var res = DropletLib.Init(getAppPath());
            if (!res)
            {
                MessageBox.Show("Failed to initialize EllipseFitLib.");
            }
            cbDropletFit.SelectedIndex = 1;
            cbBaselineFit.SelectedIndex = 0;
            cbFitMode.SelectedIndex = 0;

            resetControls();
        }

        private void configManualCtrls(bool isManual)
        {
            chkRotate.Enabled = isManual;
            chkHasPin.Enabled = isManual;
            cbHorMode.Enabled = isManual;
        }

        private void resetScale()
        {
            if (picImage.Bounds.IsEmpty)
            {
                m_scaleX = m_scaleY = 0;
                return;
            }
            m_scaleX = picImage.Image.Width / (double)picImage.Width;
            m_scaleY = picImage.Image.Height / (double)picImage.Height;
        }

        private void btnBrowse_Click(object sender, EventArgs e)
        {
            if (openDlg.ShowDialog() == DialogResult.OK)
            {
                string imagePath = openDlg.FileName;

                // Load image into PictureBox or process further
                picImage.Image = System.Drawing.Image.FromFile(imagePath);
                txtPath.Text = imagePath;
                resetControls();
                resetScale();
                init();
                m_inBaselinePts.Clear();
                m_inDropletPts.Clear();
                
                DropletLib.LoadImage(imagePath);
                if (DropletLib.GetShapeEx(out var shape, out m_horMode, out m_hasPin))
                {
                    m_shape = (MainShapeType)((int)shape % 3);
                    cbMainShape.SelectedIndex = (int)m_shape + 1;
                    cbHorMode.SelectedIndex = (int)m_horMode + 1;
                    chkHasPin.Checked = m_hasPin;
                    if (m_shape == MainShapeType.eShapeHorizontal)
                    {
                        chkManual.Enabled = true;
                    }
                    else
                    {
                        chkManual.Enabled = false;
                        chkManual.Checked = false;
                        configManualCtrls(false);
                    }
                }
                else
                {
                    var err = DropletLib.GetLastError();
                    var msg = DropletLib.GetResultToString(err);
                    MessageBox.Show($"Failed to get shape from image.({msg})");
                }

                // Reset controls to default state before processing
                // 
            }
        }

        private void resetControls()
        {
            cbMainShape.SelectedIndex = 0;
            chkSubpixel.Checked = false;

            bool isManual = false;
            chkManual.Enabled = false;
            chkManual.Checked = isManual;
            chkRotate.Checked = false;
            chkHasPin.Checked = false;
            cbHorMode.SelectedIndex = 0;

            txtP1X.Text = "0";
            txtP1Y.Text = "0";
            txtP2X.Text = "0";
            txtP2Y.Text = "0";

            configManualCtrls(isManual);
        }

        /*
         * case of old type cs api
        private void btnTest_Click(object sender, EventArgs e)
        {
            if (picImage.Image == null)
            {
                MessageBox.Show("Please select an image first.");
                return;
            }            

            string imagePath = txtPath.Text;

            // Check if the image path is valid and set the image
            var res = DropletLib.LoadImage(imagePath);
            if (!res)
            {
                MessageBox.Show("Failed to set image.");
                return;
            }

            // config parameters
            bool subPixel = chkSubpixel.Checked;
            bool rotated = false;
            bool hasPin = false;
            HorizonMode horMode = HorizonMode.eHmUnknow;
            DropletFitMode dropletFitmode = (DropletFitMode)cbDropletFit.SelectedIndex;
            BaseLineFitMode baselineFitMode = (BaseLineFitMode)cbBaselineFit.SelectedIndex;
            
            int iSelShape = cbMainShape.SelectedIndex;
            MainShapeType mainShape = MainShapeType.eShapeUnknown;
            if (iSelShape > 0)
                mainShape = (MainShapeType)(iSelShape - 1); // -1 because first item is "Auto Detect" (0 index)            
            if(mainShape == MainShapeType.eShapeHorizontal && chkManual.Checked)
            {
                rotated = chkRotate.Checked;
                hasPin = chkHasPin.Checked;
                horMode = (HorizonMode)cbHorMode.SelectedIndex-1;
            }

            // main processing
            res = DropletLib.AutoFit(dropletFitmode, baselineFitMode, mainShape, subPixel, rotated, horMode, hasPin);

            if (!res)
            {
                var err_code = DropletLib.GetLastError();
                var result = DropletLib.GetResultToString(err_code);

                MessageBox.Show($"Failed to fit image.({result})");
                return;
            }
            
            res = DropletLib.GetResult(out m_angle, out m_pointAndAngles);                        
            DropletLib.GetMidResult(out m_shape, out m_hasPin, out m_isEmpty, out m_horMode, out m_basePts, out m_boxs);            

            DropletLib.GetContourCounts(out m_nDrops, out m_nContours);            
            DropletLib.GetContours(m_nDrops, m_nContours, out m_droplets, out m_baselines);

            var shapedetail = DropletLib.GetShape();
            var detailString = DropletLib.BaseLineShapeKindToString(shapedetail);

            var jsonText = DropletLib.GetResultByJsonFormatString();
            var parsedJson = Newtonsoft.Json.Linq.JToken.Parse(jsonText);
            string formattedJson = parsedJson.ToString(Newtonsoft.Json.Formatting.Indented);
            textBox1.Text = formattedJson;

            drawResult();
        }
        */

        private void drawResult()
        {
            FitMode fitMode = (FitMode)cbFitMode.SelectedIndex;
            // Make sure the PictureBox has a valid image
            var imagePath = txtPath.Text;
            if (String.IsNullOrEmpty(imagePath))
                return;
            using (Bitmap sourceImg = (Bitmap)System.Drawing.Image.FromFile(imagePath))
            {
                Bitmap backImg = new Bitmap(sourceImg.Width, sourceImg.Height, PixelFormat.Format24bppRgb);
                if (backImg == null)
                    return;

                int w = picImage.Width;
                int h = picImage.Height;
                int imgW = backImg.Width;
                int imgH = backImg.Height;
                double scaleX = (double)w / imgW;
                double scaleY = (double)h / imgH;
                if (m_angle == null)
                {
                    string resultText =
                    $"Shape: {m_shape}\n" +
                    $"Has Pin: {m_hasPin}\n" +
                    $"Horizon Mode: {m_horMode}\n" +
                    $"Base Points: Left({m_basePts.left:F2}, {m_basePts.top:F2}), Right({m_basePts.right:F2}, {m_basePts.bottom:F2})\n";

                    // Draw directly on the image
                    using (Graphics g = Graphics.FromImage(backImg))
                    using (Pen redPen = new Pen(Color.Red, 1))
                    using (Pen bluePen = new Pen(Color.Blue, 1))
                    using (Font font = new Font("Arial", 12, FontStyle.Bold))
                    {
                        g.SmoothingMode = SmoothingMode.AntiAlias;
                        g.DrawImage(sourceImg, 0, 0, sourceImg.Width, sourceImg.Height);

                        if (chkResString.Checked)
                            g.DrawString(resultText, font, Brushes.DarkCyan, 10, 10);

                        // draw base points
                        if (chkResBasept.Checked && fitMode != FitMode.eFitManual)
                        {
                            const double mark = 5.0;
                            double x0 = m_basePts.left;
                            double y0 = m_basePts.top;

                            double x1 = m_basePts.right;
                            double y1 = m_basePts.bottom;

                            g.DrawLine(redPen, (float)(x0 - mark), (float)(y0 - mark), (float)(x0 + mark), (float)(y0 + mark));
                            g.DrawLine(redPen, (float)(x0 - mark), (float)(y0 + mark), (float)(x0 + mark), (float)(y0 - mark));

                            g.DrawLine(redPen, (float)(x1 - mark), (float)(y1 - mark), (float)(x1 + mark), (float)(y1 + mark));
                            g.DrawLine(redPen, (float)(x1 - mark), (float)(y1 + mark), (float)(x1 + mark), (float)(y1 - mark));
                        }

                        // Draw Keypoint[2];
                        if (m_keyPts != null && m_keyPts.Length > 2 && m_keyPts[2].x > 0.0f && m_keyPts[2].y > 0.0f)
                        {
                            const double mark = 5.0;
                            double x0 = m_keyPts[2].x;
                            double y0 = m_keyPts[2].y;

                            g.DrawLine(bluePen, (float)(x0 - mark), (float)(y0 - mark), (float)(x0 + mark), (float)(y0 + mark));
                            g.DrawLine(bluePen, (float)(x0 - mark), (float)(y0 + mark), (float)(x0 + mark), (float)(y0 - mark));

                        }
                    }
                }
                else
                {
                    string resultText =
                    $"Angle1: {m_angle[0]:F2}°\n" +
                    $"Angle2: {m_angle[1]:F2}°\n" +
                    $"Shape: {m_shape}\n" +
                    $"Has Pin: {m_hasPin}\n" +
                    $"Horizon Mode: {m_horMode}\n" +
                    $"Base Points: Left({m_basePts.left:F2}, {m_basePts.top:F2}), Right({m_basePts.right:F2}, {m_basePts.bottom:F2})\n";

                    // Draw directly on the image
                    using (Graphics g = Graphics.FromImage(backImg))
                    using (Pen redPen = new Pen(Color.Red, 1))
                    using (Font font = new Font("Arial", 12, FontStyle.Bold))
                    using (Pen bluePen = new Pen(Color.Blue, 1))
                    using (Pen greenPen = new Pen(Color.Green, 1))
                    {
                        g.SmoothingMode = SmoothingMode.AntiAlias;
                        g.DrawImage(sourceImg, 0, 0, sourceImg.Width, sourceImg.Height);


                        if (chkResString.Checked)
                            g.DrawString(resultText, font, Brushes.DarkCyan, 10, 10);

                        // draw points and angles
                        if (chkResAngle.Checked && m_pointAndAngles.Length >= 2)
                        {
                            const double r = 50.0;
                            double x0 = m_pointAndAngles[0].point.x;
                            double y0 = m_pointAndAngles[0].point.y;
                            double dx01 = m_pointAndAngles[0].vec1.x * r;
                            double dy01 = m_pointAndAngles[0].vec1.y * r;
                            double dx02 = m_pointAndAngles[0].vec2.x * r;
                            double dy02 = m_pointAndAngles[0].vec2.y * r;

                            double x1 = m_pointAndAngles[1].point.x;
                            double y1 = m_pointAndAngles[1].point.y;
                            double dx11 = m_pointAndAngles[1].vec1.x * r;
                            double dy11 = m_pointAndAngles[1].vec1.y * r;
                            double dx12 = m_pointAndAngles[1].vec2.x * r;
                            double dy12 = m_pointAndAngles[1].vec2.y * r;
                            if (x0 != 0 && y0 != 0)
                            {
                                g.DrawLine(redPen, (float)x0, (float)y0, (float)(x0 + dx01), (float)(y0 + dy01));
                                g.DrawLine(redPen, (float)x0, (float)y0, (float)(x0 + dx02), (float)(y0 + dy02));
                            }

                            if (x1 != 0 && y1 != 0)
                            {
                                g.DrawLine(bluePen, (float)x1, (float)y1, (float)(x1 + dx11), (float)(y1 + dy11));
                                g.DrawLine(bluePen, (float)x1, (float)y1, (float)(x1 + dx12), (float)(y1 + dy12));
                            }
                        }

                        // draw base points
                        if (chkResBasept.Checked && fitMode != FitMode.eFitManual)
                        {
                            const double mark = 5.0;
                            double x0 = m_basePts.left;
                            double y0 = m_basePts.top;

                            double x1 = m_basePts.right;
                            double y1 = m_basePts.bottom;

                            g.DrawLine(redPen, (float)(x0 - mark), (float)(y0 - mark), (float)(x0 + mark), (float)(y0 + mark));
                            g.DrawLine(redPen, (float)(x0 - mark), (float)(y0 + mark), (float)(x0 + mark), (float)(y0 - mark));

                            g.DrawLine(redPen, (float)(x1 - mark), (float)(y1 - mark), (float)(x1 + mark), (float)(y1 + mark));
                            g.DrawLine(redPen, (float)(x1 - mark), (float)(y1 + mark), (float)(x1 + mark), (float)(y1 - mark));

                            // Draw Keypoint[2];
                            if (m_keyPts != null && m_keyPts.Length > 2 && m_keyPts[2].x > 0.0f && m_keyPts[2].y > 0.0f)
                            {
                                x0 = m_keyPts[2].x;
                                y0 = m_keyPts[2].y;

                                g.DrawLine(bluePen, (float)(x0 - mark), (float)(y0 - mark), (float)(x0 + mark), (float)(y0 + mark));
                                g.DrawLine(bluePen, (float)(x0 - mark), (float)(y0 + mark), (float)(x0 + mark), (float)(y0 - mark));

                            }
                        }
                        if (chkResContour.Checked)
                        {
                            if (m_boxs != null)
                            {
                                Pen[] selPen = { bluePen, greenPen, redPen };
                                int nBox = Math.Min(3, m_boxs.Length);
                                for (int i = 0; i < nBox; i++)
                                {
                                    if (m_boxs[i].center.x != 0 && m_boxs[i].center.y != 0)
                                    {
                                        g.ResetTransform();
                                        g.TranslateTransform((float)m_boxs[i].center.x, (float)m_boxs[i].center.y);
                                        g.RotateTransform((float)m_boxs[i].angle);
                                        if (i == 0 && m_fgType == FittingGraphicsType.eFgRectangle)
                                            g.DrawRectangle(selPen[i], (float)(-m_boxs[i].a / 2), (float)(-m_boxs[i].b / 2), (float)(m_boxs[i].a), (float)(m_boxs[i].b));
                                        else
                                            g.DrawEllipse(selPen[i], (float)(-m_boxs[i].a), (float)(-m_boxs[i].b), (float)(m_boxs[i].a * 2), (float)(m_boxs[i].b * 2));
                                        g.ResetTransform();
                                    }
                                }
                            }
                            if (m_leftFitRes != null)
                            {
                                int nLeft = m_leftFitRes.Length;
                                for (int i = 1; i < nLeft; i++)
                                {
                                    g.DrawLine(bluePen, (float)m_leftFitRes[i - 1].x, (float)m_leftFitRes[i - 1].y, (float)m_leftFitRes[i].x, (float)m_leftFitRes[i].y);
                                }
                            }
                            if (m_rightFitRes != null)
                            {
                                int nLeft = m_rightFitRes.Length;
                                for (int i = 1; i < nLeft; i++)
                                {
                                    g.DrawLine(greenPen, (float)m_rightFitRes[i - 1].x, (float)m_rightFitRes[i - 1].y, (float)m_rightFitRes[i].x, (float)m_rightFitRes[i].y);
                                }
                            }
                        }
                    }
                }
                // Refresh the PictureBox to show the updated image
                if (picImage.Image != null)
                    picImage.Image.Dispose();
                picImage.Image = backImg;
            }
        }

        private void drawInputPoints(PaintEventArgs e)
        {
            foreach (var p in m_inBaselinePts)
            {
                e.Graphics.FillEllipse(Brushes.Red, p.X - pointRadius, p.Y - pointRadius, pointRadius * 2, pointRadius * 2);
                e.Graphics.DrawEllipse(Pens.Red, p.X - pointRadius, p.Y - pointRadius, pointRadius * 2, pointRadius * 2);
            }

            // Draw right (blue)
            foreach (var p in m_inDropletPts)
            {
                e.Graphics.FillEllipse(Brushes.Blue, p.X - pointRadius, p.Y - pointRadius, pointRadius * 2, pointRadius * 2);
                e.Graphics.DrawEllipse(Pens.Blue, p.X - pointRadius, p.Y - pointRadius, pointRadius * 2, pointRadius * 2);
            }
        }

        private void drawDesign()
        {
            // Make sure the PictureBox has a valid image
            var imagePath = txtPath.Text;
            if (String.IsNullOrEmpty(imagePath))
                return;
            using (Bitmap sourceImg = (Bitmap)System.Drawing.Image.FromFile(imagePath))
            {
                Bitmap backImg = new Bitmap(sourceImg.Width, sourceImg.Height, PixelFormat.Format24bppRgb);
                
                if (backImg == null)
                    return;

                FitMode fitMode = (FitMode)cbFitMode.SelectedIndex;
                if (fitMode == FitMode.eFitAuto)
                    return;

                int w = picImage.Width;
                int h = picImage.Height;
                int imgW = backImg.Width;
                int imgH = backImg.Height;
                double scaleX = (double)w / imgW;
                double scaleY = (double)h / imgH;
                using (Graphics g = Graphics.FromImage(backImg))
                {
                    g.DrawImage(sourceImg, 0, 0, sourceImg.Width, sourceImg.Height);
                    foreach (var p in m_inBaselinePts)
                    {
                        int x = (int)(p.X * m_scaleX);
                        int y = (int)(p.Y * m_scaleY);
                        g.FillEllipse(Brushes.Red, x - pointRadius, y - pointRadius, pointRadius * 2, pointRadius * 2);
                        g.DrawEllipse(Pens.Red, x - pointRadius, y - pointRadius, pointRadius * 2, pointRadius * 2);
                    }

                    // Draw right (blue)
                    foreach (var p in m_inDropletPts)
                    {
                        int x = (int)(p.X * m_scaleX);
                        int y = (int)(p.Y * m_scaleY);
                        g.FillEllipse(Brushes.Blue, x - pointRadius, y - pointRadius, pointRadius * 2, pointRadius * 2);
                        g.DrawEllipse(Pens.Blue, x - pointRadius, y - pointRadius, pointRadius * 2, pointRadius * 2);
                    }
                }
                // Refresh the PictureBox to show the updated image
                if (picImage.Image != null)
                    picImage.Image.Dispose();                
                picImage.Image = backImg;
            }
        }

        private void FrmMain_FormClosing(object sender, FormClosingEventArgs e)
        {
            DropletLib.Release();
        }

        private void chkManual_CheckedChanged(object sender, EventArgs e)
        {
            configManualCtrls(chkManual.Checked);
            m_bDesignTime = true;
        }


        private void cbMainShape_SelectedIndexChanged(object sender, EventArgs e)
        {
            int iSel = cbMainShape.SelectedIndex;
            bool validManual = false;
            bool isManual = false;
            if (iSel == (int)MainShapeType.eShapeHorizontal + 1)
            {
                validManual = true;
                isManual = chkManual.Checked;
            }
            else
            {
                validManual = false;
                chkManual.Enabled = false;
                isManual = false;
            }
            chkManual.Enabled = validManual;
            configManualCtrls(isManual);
            m_bDesignTime = true;

            m_shape = (MainShapeType)(iSel - 1);
        }

        private void chkResString_CheckedChanged(object sender, EventArgs e)
        {
            drawResult();
        }

        private void chkResAngle_CheckedChanged(object sender, EventArgs e)
        {
            drawResult();
        }

        private void chkResBasept_CheckedChanged(object sender, EventArgs e)
        {
            drawResult();
        }

        private void chkResContour_CheckedChanged(object sender, EventArgs e)
        {
            drawResult();
        }

        private void exampleOnManual()
        {
            // 输入参数
            bool isSubPixel = false;
            DropletFitMode upFitMode = DropletFitMode.eDropletEllipse;
            BaseLineFitMode downFitMode = BaseLineFitMode.eBaseLineCircle;
            DropletLibCSharp.PointF[] ptsOnbaseline = new DropletLibCSharp.PointF[2]; // 基线上的点(2个点)
            ptsOnbaseline[0].y = 100.0f; ptsOnbaseline[0].x = 150.0f;
            ptsOnbaseline[1].y = 100.0f; ptsOnbaseline[1].x = 350.0f;
            DropletLibCSharp.PointF[] ptsOnDroplet = new DropletLibCSharp.PointF[2]; // 滴液上的点(2个点)
            ptsOnDroplet[0].y = 50.0f; ptsOnDroplet[0].x = 200.0f;
            ptsOnDroplet[1].y = 80.0f; ptsOnDroplet[1].x = 300.0f;

            bool res = false;
            // 1. 图片路径
            string imagePath = txtPath.Text;
            res = DropletLib.LoadImage(imagePath);

            // 2. 获取基线形状
            BaseLineShapeKind detailedShape;
            HorizonMode horMode = HorizonMode.eHmUnknow;
            bool hasPin = false;
            DropletLib.GetShapeEx(out detailedShape, out horMode, out hasPin);
            MainShapeType mainShape = (MainShapeType)((int)detailedShape % 3);

            //3. 拟合
            FittingInfoCS fitRes;
            if (mainShape == MainShapeType.eShapeHorizontal && upFitMode == DropletFitMode.eDropletWidthHeight)
            {
                res = DropletLib.ManualFitting(
                mainShape, upFitMode, downFitMode,
                ptsOnDroplet.Length, ptsOnDroplet, // 滴液矩形的2个点
                0, null,
                hasPin,
                out fitRes,
                isSubPixel);
            }
            else
            {
                res = DropletLib.ManualFitting(
                mainShape, upFitMode, downFitMode,
                ptsOnDroplet.Length, ptsOnDroplet,
                ptsOnbaseline.Length, ptsOnbaseline,
                hasPin,
                out fitRes,
                isSubPixel);
            }

            if (res)
            {
                // TODO: 处理拟合结果
            }
        }
        private void exampleOnSemiAuto()
        {
            // 输入参数
            bool isSubPixel = false;
            DropletLibCSharp.PointF[] ptsOnbaseline = new DropletLibCSharp.PointF[5]; // 基线上的点(2, 3, 5个点)
            ptsOnbaseline[0].y = 100.0f; ptsOnbaseline[0].x = 150.0f;
            ptsOnbaseline[1].y = 130.0f; ptsOnbaseline[1].x = 350.0f;
            ptsOnbaseline[2].y = 120.0f; ptsOnbaseline[2].x = 104.0f;
            ptsOnbaseline[3].y = 110.0f; ptsOnbaseline[3].x = 285.0f;
            ptsOnbaseline[4].y = 120.0f; ptsOnbaseline[4].x = 294.0f;

            DropletFitMode upFitMode = DropletFitMode.eDropletEllipse;
            BaseLineFitMode downFitMode = BaseLineFitMode.eBaseLineCircle;

            bool res = false;
            // 1. 图片路径
            string imagePath = txtPath.Text;
            res = DropletLib.LoadImage(imagePath);

            // 2. 获取基线形状
            BaseLineShapeKind detailedShape;
            HorizonMode horMode = HorizonMode.eHmUnknow;
            bool hasPin = false;
            DropletLib.GetShapeEx(out detailedShape, out horMode, out hasPin);
            MainShapeType mainShape = (MainShapeType)((int)detailedShape % 3);

            // 3. 获取基线点   
            DropletLibCSharp.RectangleF basePt;
            if (mainShape == MainShapeType.eShapeHorizontal)
            {
                // 在这种情况下，输入两个点（直线上的两个点）。
                DropletLibCSharp.PointF[] pBasePts = new DropletLibCSharp.PointF[2];
                pBasePts[0] = ptsOnbaseline[0];
                pBasePts[1] = ptsOnbaseline[1];
                res = DropletLib.GetBasepointsBylineOnHorizontal(pBasePts, out basePt, isSubPixel);
            }
            else
            {
                // 如果拟合是圆: 输入3个点，椭圆: 输入5个点。
                res = DropletLib.GetBasepointsBySemiautoOnSurface(mainShape, ptsOnbaseline.Length, ptsOnbaseline, out basePt, isSubPixel);
            }

            // 4. 拟合
            FittingInfoCS fitRes;
            res = DropletLib.SemiautoFitting(
                mainShape,
                upFitMode,
                downFitMode,
                basePt,
                ptsOnbaseline.Length,
                ptsOnbaseline,
                out fitRes
            );
            if (res)
            {
                // TODO: 处理拟合结果
            }
        }
        private void examplesOnBypointsAndByline()
        {
            // 输入参数
            bool isSubPixel = false;
            DropletLibCSharp.PointF[] basePtbyUser = new DropletLibCSharp.PointF[2]; // 基线点位置或基线直线上的两点
            basePtbyUser[0].y = 100.0f; basePtbyUser[0].x = 150.0f;
            basePtbyUser[1].y = 100.0f; basePtbyUser[1].x = 350.0f;

            DropletFitMode upFitMode = DropletFitMode.eDropletEllipse;
            BaseLineFitMode downFitMode = BaseLineFitMode.eBaseLineCircle;

            bool res = false;
            // 1. 图片路径
            string imagePath = txtPath.Text;
            res = DropletLib.LoadImage(imagePath);

            // 2. 获取基线形状
            BaseLineShapeKind detailedShape;
            HorizonMode horMode = HorizonMode.eHmUnknow;
            bool hasPin = false;
            DropletLib.GetShapeEx(out detailedShape, out horMode, out hasPin);
            MainShapeType mainShape = (MainShapeType)((int)detailedShape % 3);

            // 3. 获取基线点   
            DropletLibCSharp.RectangleF basePt;
            res = false;
            if (mainShape == MainShapeType.eShapeHorizontal) // 水平面？
            {
                res = DropletLib.GetBasepointsByPointsOnHorizontal(basePtbyUser, out basePt, isSubPixel);
                // res = DropletLib.GetBasepointsBylineOnHorizontal(basePtbyUser, isSubPixel, out basePt);
            }
            else
            {
                res = DropletLib.GetBasepointsByPointsOnSurface(basePtbyUser, out basePt, isSubPixel);
                // res = DropletLib.GetBasepointsBylineOnSurface(basePtbyUser, isSubPixel, out basePt);
            }

            // 4. 拟合
            FittingInfoCS fitRes;
            res = DropletLib.PointsFitting(upFitMode, downFitMode, basePt, out fitRes);
            if (res)
            {
                // TODO: 处理拟合结果
            }
        }

        private void btnRecog_Click(object sender, EventArgs e)
        {
            FitMode fitMode = (FitMode)cbFitMode.SelectedIndex;
            bool isHorizontal = (m_shape == MainShapeType.eShapeHorizontal);
            DropletFitMode dropletFitMode = (DropletFitMode)cbDropletFit.SelectedIndex;
            BaseLineFitMode baselineFitMode = (BaseLineFitMode)cbBaselineFit.SelectedIndex;
            FittingInfoCS outInfo = new FittingInfoCS();
            bool res = false;
            if (fitMode == FitMode.eFitAuto)
            {
                if(isHorizontal)
                    res = DropletLib.AutoFittingOnHorizontal(dropletFitMode, m_basePts, out outInfo);
                else
                    res = DropletLib.AutoFittingOnSurface(m_shape, dropletFitMode, baselineFitMode, m_basePts, out outInfo);
            }
            else if(fitMode == FitMode.eFitByBasePoint || fitMode == FitMode.eFitByLine)
            {
                res = DropletLib.PointsFitting(dropletFitMode, baselineFitMode, m_basePts, out outInfo);
            }
            else if(fitMode == FitMode.eFitSemiAuto)
            {
                DropletLibCSharp.PointF[] pts = new DropletLibCSharp.PointF[m_inBaselinePts.Count];
                for(int i = 0; i < m_inBaselinePts.Count; i++)
                {
                    pts[i].x = (float)(m_inBaselinePts[i].X * m_scaleX);
                    pts[i].y = (float)(m_inBaselinePts[i].Y * m_scaleY);
                }
                res = DropletLib.SemiautoFitting(
                    m_shape,
                    dropletFitMode,
                    baselineFitMode,
                    m_basePts,
                    pts.Length,
                    pts,
                    out outInfo
                );
            }
            else
            {
                DropletLibCSharp.PointF[] ptsBaseline = new DropletLibCSharp.PointF[m_inBaselinePts.Count];
                for (int i = 0; i < m_inBaselinePts.Count; i++)
                {
                    ptsBaseline[i].x = (float)(m_inBaselinePts[i].X * m_scaleX);
                    ptsBaseline[i].y = (float)(m_inBaselinePts[i].Y * m_scaleY);
                }

                DropletLibCSharp.PointF[] ptsDroplet = new DropletLibCSharp.PointF[m_inDropletPts.Count];
                for (int i = 0; i < m_inDropletPts.Count; i++)
                {
                    ptsDroplet[i].x = (float)(m_inDropletPts[i].X * m_scaleX);
                    ptsDroplet[i].y = (float)(m_inDropletPts[i].Y * m_scaleY);
                }
                bool subPixel = chkSubpixel.Checked;
                bool hasPin = chkHasPin.Checked;
                res = DropletLib.ManualFitting(m_shape, dropletFitMode, baselineFitMode, 
                    ptsDroplet.Length, ptsDroplet, 
                    ptsBaseline.Length, ptsBaseline, 
                    hasPin, 
                    out outInfo,
                    subPixel);
            }
            
            m_angle = outInfo.angle;
            m_pointAndAngles = outInfo.pointAngle;
            m_boxs = outInfo.boxes;
            m_leftFitRes = outInfo.leftPoints;
            m_rightFitRes = outInfo.rightPoints;
            m_fgType = outInfo.fgType;

            var jsonText = DropletLib.GetResultByJsonFormatString();
            var parsedJson = Newtonsoft.Json.Linq.JToken.Parse(jsonText);
            string formattedJson = parsedJson.ToString(Newtonsoft.Json.Formatting.Indented);
            textBox1.Text = formattedJson;
            m_bDesignTime = false;

            drawResult();

            if(!res)
            {
                var errcode = DropletLib.GetLastError();
                var result = DropletLib.GetResultToString(errcode);
                MessageBox.Show(result, $"Error({result})", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void btnGetbasePts_Click(object sender, EventArgs e)
        {
            if (m_shape == MainShapeType.eShapeUnknown)
            {
                return;
            }
            init();

            int iFitMode = cbFitMode.SelectedIndex;
            FitMode fitMode = (FitMode)iFitMode;
            bool subPixel = chkSubpixel.Checked;
            bool isBounday = chkSupplyBoundary.Checked;
            bool hasPin = chkHasPin.Checked;
            bool res = false;
            if (fitMode == FitMode.eFitAuto)
            {
                if (m_shape == MainShapeType.eShapeHorizontal)
                {                    
                    bool rotated = chkRotate.Checked;
                    HorizonMode horMode = HorizonMode.eHmUnknow;
                    if (chkManual.Checked)
                        horMode = (HorizonMode)cbHorMode.SelectedIndex-1;

                    res = DropletLib.GetBasepointsByAutoOnHorizontal(
                        ref hasPin,
                        ref rotated,
                        ref horMode,                        
                        out m_basePts,
                        m_keyPts,
                        subPixel,
                        isBounday);
                }
                else
                {
                    res = DropletLib.GetBasepointsByAutoOnSurface(hasPin, out m_basePts, subPixel, isBounday);
                }
            }
            else if (fitMode == FitMode.eFitByBasePoint)
            {
                if (m_shape == MainShapeType.eShapeHorizontal)
                {
                    DropletLibCSharp.PointF[] pts = new DropletLibCSharp.PointF[2];
                    for (int i = 0; i < 2; i++)
                    {
                        pts[i].x = (double)m_inBaselinePts[i].X * m_scaleX;
                        pts[i].y = (double)m_inBaselinePts[i].Y * m_scaleY;
                    }

                    res = DropletLib.GetBasepointsByPointsOnHorizontal(pts, out m_basePts, subPixel);
                }
                else
                {
                    DropletLibCSharp.PointF[] pts = new DropletLibCSharp.PointF[2];
                    for (int i = 0; i < 2; i++)
                    {
                        pts[i].x = (double)m_inBaselinePts[i].X * m_scaleX;
                        pts[i].y = (double)m_inBaselinePts[i].Y * m_scaleY;
                    }
                    res = DropletLib.GetBasepointsByPointsOnSurface(pts, out m_basePts, subPixel);
                }
            }
            else if(fitMode == FitMode.eFitByLine)
            {
                if (m_shape == MainShapeType.eShapeHorizontal)
                {
                    DropletLibCSharp.PointF[] pts = new DropletLibCSharp.PointF[2];
                    for (int i = 0; i < 2; i++)
                    {
                        pts[i].x = (double)m_inBaselinePts[i].X * m_scaleX;
                        pts[i].y = (double)m_inBaselinePts[i].Y * m_scaleY;
                    }

                    res = DropletLib.GetBasepointsBylineOnHorizontal(pts, out m_basePts, subPixel);
                }
                else
                {
                    DropletLibCSharp.PointF[] pts = new DropletLibCSharp.PointF[2];
                    for (int i = 0; i < 2; i++)
                    {
                        pts[i].x = (double)m_inBaselinePts[i].X * m_scaleX;
                        pts[i].y = (double)m_inBaselinePts[i].Y * m_scaleY;
                    }
                    res = DropletLib.GetBasepointsBylineOnSurface(pts, out m_basePts, subPixel);
                }
            }
            else if(fitMode == FitMode.eFitSemiAuto)
            {
                if (m_shape == MainShapeType.eShapeHorizontal)
                {
                    DropletLibCSharp.PointF[] pts = new DropletLibCSharp.PointF[2];
                    for (int i = 0; i < 2; i++)
                    {
                        pts[i].x = (double)m_inBaselinePts[i].X * m_scaleX;
                        pts[i].y = (double)m_inBaselinePts[i].Y * m_scaleY;
                    }

                    res = DropletLib.GetBasepointsBylineOnHorizontal(pts, out m_basePts, subPixel);
                }
                else
                {
                    int n = m_inBaselinePts.Count;
                    DropletLibCSharp.PointF[] pts = new DropletLibCSharp.PointF[n];
                    for (int i = 0; i < n; i++)
                    {
                        pts[i].x = (double)m_inBaselinePts[i].X * m_scaleX;
                        pts[i].y = (double)m_inBaselinePts[i].Y * m_scaleY;
                    }
                    res = DropletLib.GetBasepointsBySemiautoOnSurface(m_shape, pts.Length, pts, out m_basePts, subPixel);
                }
            }
            if(res)
            {
                txtP1X.Text = m_basePts.left.ToString("F2");
                txtP1Y.Text = m_basePts.top.ToString("F2");
                txtP2X.Text = m_basePts.right.ToString("F2");
                txtP2Y.Text = m_basePts.bottom.ToString("F2");
            }
            else
            {
                //try
                //{
                    var err_code = DropletLib.GetLastError();
                    var result = DropletLib.GetResultToString(err_code);
                    String msg = $"Failed to get base points.({result})";
                    //this.Activate();
                    //this.BringToFront();
                    MessageBox.Show(this, msg, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                //}
                //catch (Exception ex)
                //{
                //    MessageBox.Show($"Failed to get base points.({ex.Message})");
                //}
            }
            m_bDesignTime = false;
            picImage.Invalidate();
        }

        private void picImage_Resize(object sender, EventArgs e)
        {
            resetScale();
        }

        private void cbFitMode_SelectedIndexChanged(object sender, EventArgs e)
        {
            m_bDesignTime = true;

            int iFitMode = cbFitMode.SelectedIndex;
            FitMode fitMode = (FitMode)iFitMode;

            bool manual = (fitMode == FitMode.eFitManual);

            btnGetbasePts.Enabled = !manual;

            m_inBaselinePts.Clear();
            m_inDropletPts.Clear();
            picImage.Invalidate();
        }

        private bool IsValidMouseEvent()
        {
            bool res = false;
            if (picImage.Image == null)
                return res;
            if (m_scaleX == 0 || m_scaleY == 0)
                return res;
            int iFitMode = cbFitMode.SelectedIndex;
            FitMode fitMode = (FitMode)iFitMode;
            if (fitMode == FitMode.eFitAuto)
                return res;
            res = true;
            return res;
        }

        private int IsNearPoint(Point[] array, Point pt)
        {
            int idx = -1;
            const int threshold = 5 * 5;
            for (int i = 0; i < array.Length; i++)
            {
                var p = array[i];
                double dist = (p.X - pt.X) * (p.X - pt.X) + (p.Y - pt.Y) * (p.Y - pt.Y);
                if (dist < threshold)
                {
                    idx = i;
                    break;
                }
            }
            return idx;
        }

        private void picImage_MouseDown(object sender, MouseEventArgs e)
        {
            int iFitMode = cbFitMode.SelectedIndex;
            FitMode fitMode = (FitMode)iFitMode;
            DropletFitMode dropletFitmode = (DropletFitMode)cbDropletFit.SelectedIndex;
            BaseLineFitMode baselineFitMode = (BaseLineFitMode)cbBaselineFit.SelectedIndex;

            if (fitMode == FitMode.eFitAuto) 
                return;
            int leftMode = 2, rightMode = 0;
            if (fitMode == FitMode.eFitByLine || fitMode == FitMode.eFitByBasePoint)
            {
                leftMode = 2;
            }
            else if (fitMode == FitMode.eFitSemiAuto)
            {
                if (m_shape == MainShapeType.eShapeHorizontal)
                {
                    leftMode = 2;
                }
                else
                {
                    if (baselineFitMode == BaseLineFitMode.eBaseLineCircle)
                        leftMode = 3;
                    else
                        leftMode = 5;
                }
            }
            else if (fitMode == FitMode.eFitManual)
            {
                if (m_shape == MainShapeType.eShapeHorizontal)
                {
                    leftMode = 0;
                }
                else
                {
                    if (baselineFitMode == BaseLineFitMode.eBaseLineCircle)
                        leftMode = 3;
                    else
                        leftMode = 5;
                }

                if (dropletFitmode == DropletFitMode.eDropletCircle)
                    rightMode = 3;
                else
                    rightMode = 5;
            }

            if (m_inBaselinePts.Count > leftMode)
                m_inBaselinePts = m_inBaselinePts.GetRange(0, leftMode);
            if (m_inDropletPts.Count > rightMode)
                m_inDropletPts = m_inDropletPts.GetRange(0, rightMode);

            if (e.Button == MouseButtons.Left)
            {
                // Try to start dragging first (any point near mouse)
                if (!TryStartDragging(e.Location, m_inBaselinePts, true) &&
                    !TryStartDragging(e.Location, m_inDropletPts, false))
                {
                    // Not near any point → add red (if under limit)
                    if (m_inBaselinePts.Count < leftMode)
                        m_inBaselinePts.Add(e.Location);
                }

            }
            else if (e.Button == MouseButtons.Right)
            {
                // Right click → add blue points (if under limit)
                if (m_inDropletPts.Count < rightMode)
                    m_inDropletPts.Add(e.Location);
            }
            //UpdateBasePointsTextBox();
            picImage.Invalidate();

        }

        private bool TryStartDragging(Point location, List<Point> pointList, bool isLeft)
        {
            for (int i = 0; i < pointList.Count; i++)
            {
                if (IsNearPoint(location, pointList[i]))
                {
                    draggingIndex = i;
                    draggingLeft = isLeft;
                    return true;
                }
            }
            return false;
        }

        private bool IsNearPoint(Point mouse, Point p)
        {
            int dx = mouse.X - p.X;
            int dy = mouse.Y - p.Y;
            int distSq = dx * dx + dy * dy;

            int threshold = (pointRadius + 10) * (pointRadius + 10); // allow a margin
            return distSq <= threshold;
        }

        private bool IsNearAnyPoint(Point mouse)
        {
            foreach (var p in m_inBaselinePts)
                if (IsNearPoint(mouse, p)) return true;

            foreach (var p in m_inDropletPts)
                if (IsNearPoint(mouse, p)) return true;

            return false;
        }

        private void UpdateBasePointsTextBox()
        {
            if (m_bDesignTime)
            {
                if (m_inBaselinePts.Count == 2)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        if (m_inBaselinePts[i].X > 0 && m_inBaselinePts[i].Y > 0)
                        {
                            var x = (double)m_inBaselinePts[i].X * m_scaleX;
                            var y = (double)m_inBaselinePts[i].Y * m_scaleY;
                            if (i == 0)
                            {
                                txtP1X.Text = x.ToString("F2");
                                txtP1Y.Text = y.ToString("F2");
                            }
                            else if (i == 1)
                            {
                                txtP2X.Text = x.ToString("F2");
                                txtP2Y.Text = y.ToString("F2");
                            }
                        }
                    }
                }
            }
        }
        private void picImage_MouseMove(object sender, MouseEventArgs e)
        {
            if (draggingIndex >= 0 && e.Button == MouseButtons.Left)
            {
                // Move the point being dragged
                if (draggingLeft)
                    m_inBaselinePts[draggingIndex] = e.Location;
                else
                    m_inDropletPts[draggingIndex] = e.Location;

                picImage.Invalidate();
            }
            else
            {
                // Change cursor if near any point
                if (IsNearAnyPoint(e.Location))
                    picImage.Cursor = Cursors.SizeAll;
                else
                    picImage.Cursor = Cursors.Default;
            }
        }

        private void picImage_MouseUp(object sender, MouseEventArgs e)
        {
            draggingIndex = -1;        
        }

        private void picImage_Paint(object sender, PaintEventArgs e)
        {            
            if(m_bDesignTime)
                drawDesign();
            else
                drawResult();
        }

        private void chkHasPin_CheckedChanged(object sender, EventArgs e)
        {
            m_bDesignTime = true;
        }

        private void chkSubpixel_CheckedChanged(object sender, EventArgs e)
        {
            m_bDesignTime = true;
        }

        private void chkSupplyBoundary_CheckedChanged(object sender, EventArgs e)
        {
            m_bDesignTime = true;
        }

        private void chkRotate_CheckedChanged(object sender, EventArgs e)
        {
            m_bDesignTime = true;
        }

        private void cbHorMode_SelectedIndexChanged(object sender, EventArgs e)
        {
            m_bDesignTime = true;
        }

        private void cbDropletFit_SelectedIndexChanged(object sender, EventArgs e)
        {
            m_bDesignTime = true;
        }

        private void cbBaselineFit_SelectedIndexChanged(object sender, EventArgs e)
        {
            m_bDesignTime = true;
        }
    }
}

