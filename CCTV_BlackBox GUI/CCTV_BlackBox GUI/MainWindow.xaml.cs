using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Forms;
using System.Windows.Threading;
using System.Text.RegularExpressions;
using System.Diagnostics;
using System.Threading;
using System.Media;
using MahApps.Metro.Controls;
using MahApps.Metro.Controls.Dialogs;
using Notifications.Wpf;

using MessageBox = System.Windows.Forms.MessageBox;
using TextBox = System.Windows.Controls.TextBox;

namespace CCTV_BlackBox_GUI
{
    /// <summary>
    /// MainWindow.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class MainWindow : MetroWindow
    {
        NotifyIcon notify = new NotifyIcon();
        Process baseProcess = new Process();
        SoundPlayer alarmPlayer = new SoundPlayer(Properties.Resources.alarm);
        SoundPlayer errorPlayer = new SoundPlayer(Properties.Resources.error);
        String basePath = @"C:\Program Files\CCTVBlackBox\";
        String imagePath = @"C:\Program Files\CCTVBlackBox\image";
        Mutex mutex;
        bool shutdownProcess = false;

        public MainWindow()
        {
            bool createdNew;
            mutex = new Mutex(true, "CCTVBlackBox", out createdNew);
            if (!createdNew)
            {
                MessageBox.Show("프로그램이 이미 실행되고 있습니다.", "중복 실행");
                shutdownProcess = true;
                System.Windows.Application.Current.Shutdown();
                return;
            }

            InitializeComponent();
            InitApp();
        }

        private void InitApp()
        {
            if (shutdownProcess || !InitBaseProcess())
                return;
            InitTrayIcon();
            alarmPlayer.Load();
            errorPlayer.Load();
            RefreshInfo();
        }

        private bool InitBaseProcess()
        {
            //if (!File.Exists(basePath))
            //    Directory.CreateDirectory(basePath);
            //String extractPath = basePath + "CCTVBlackBoxCLI.exe";
            //File.WriteAllBytes(extractPath, Properties.Resources.CCTVBlackBoxCLI);

            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.FileName = "CCTVBlackBoxCLI.exe"; //extractPath
            startInfo.CreateNoWindow = true;
            startInfo.UseShellExecute = false;
            startInfo.RedirectStandardInput = true;
            startInfo.RedirectStandardOutput = true;
            startInfo.RedirectStandardError = false;

            baseProcess.StartInfo = startInfo;
            baseProcess.OutputDataReceived += proc_OutputDataReceived;
            baseProcess.EnableRaisingEvents = true;
            baseProcess.Exited += BaseProcess_Exited;
            try
            {
                baseProcess.Start();
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message, "기반 프로세스 실행 오류");
                shutdownProcess = true;
                System.Windows.Application.Current.Shutdown();
                return false;
            }
            baseProcess.BeginOutputReadLine();

            return true;
        }

        private void proc_OutputDataReceived(object sender, DataReceivedEventArgs e)
        {
            if (String.IsNullOrEmpty(e.Data))
            {
                return; 
            }
            String[] split = e.Data.Split('|');
            String type = split[0];
            String data = 1 < split.Length ? split[1] : null;
            if (type.Equals("CHANGE_DETECTION"))
            {
                alarmPlayer.Play();
                var notificationManager = new NotificationManager();
                notificationManager.Show(new NotificationContent
                {
                    Title = "변화 감지",
                    Message = "모니터에 변화가 감지되었습니다.",
                    Type = NotificationType.Warning
                });

            } else if (type.Equals("INFO"))
            {
                String[] values = data.Split(',');
                int timerInterval = Int32.Parse(values[0]);
                int alarmInterval = Int32.Parse(values[1]);
                String[] saveInterval = values[2].Split(' ');
                int saveIntervalMin = Int32.Parse(saveInterval[0]);
                int saveIntervalMax = Int32.Parse(saveInterval[1]);
                int cornerSize = Int32.Parse(values[3]);
                float threshold = float.Parse(values[4]);
                float matching = float.Parse(values[5]);
                int debug = Int32.Parse(values[6]);
                int alarm = Int32.Parse(values[7]);
                int deleteInterval = Int32.Parse(values[8]);

                this.Dispatcher.Invoke((Action)(() =>
                {
                    TimerIntervalInput.Value = timerInterval;
                    AlarmIntervalInput.Value = alarmInterval;
                    SaveIntervalMinInput.Value = saveIntervalMin;
                    SaveIntervalMaxInput.Value = saveIntervalMax;
                    CornerSizeInput.Value = cornerSize;
                    ThresholdSlider.Value = threshold;
                    MatchingSlider.Value = matching;
                    DebugCheck.IsChecked = Convert.ToBoolean(debug);
                    AlarmCheck.IsChecked = Convert.ToBoolean(alarm);
                    DeleteIntervalInput.Value = deleteInterval;
                }));
            }
        }

        private void InitTrayIcon()
        {
            System.Windows.Forms.ContextMenu menu = new System.Windows.Forms.ContextMenu();

            System.Windows.Forms.MenuItem item1 = new System.Windows.Forms.MenuItem();
            item1.Index = 0;
            item1.Text = "열기";
            item1.Click += delegate (object click, EventArgs eClick)
            {
                this.Show();
                this.WindowState = WindowState.Normal;
                this.Visibility = Visibility.Visible;
            };
            System.Windows.Forms.MenuItem item2 = new System.Windows.Forms.MenuItem();
            item2.Index = 1;
            item2.Text = "프로그램 종료";
            item2.Click += delegate (object click, EventArgs eClick)
            {
                shutdownProcess = true;
                notify.Dispose();
                baseProcess.StandardInput.Close();
                baseProcess.Kill();
                baseProcess.WaitForExit();
                mutex.ReleaseMutex();
                System.Windows.Application.Current.Shutdown();
            };


            menu.MenuItems.Add(item1);
            menu.MenuItems.Add(item2);

            notify.Icon = Properties.Resources.cctv;
            notify.Visible = true;
            notify.DoubleClick += delegate (object senders, EventArgs args)
            {
                this.Show();
                this.WindowState = WindowState.Normal;
                this.Visibility = Visibility.Visible;
            };
            notify.ContextMenu = menu;
            notify.Text = "CCTV BlackBox";
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            e.Cancel = true;
            this.Hide();
            base.OnClosing(e);
        }

        private void BaseProcess_Exited(object sender, System.EventArgs e)
        {
            if (shutdownProcess)
                return;
            errorPlayer.Play();
            var notificationManager = new NotificationManager();
            notificationManager.Show(new NotificationContent
            {
                Title = "오류 발생",
                Message = "기반 프로세스가 종료되었습니다.",
                Type = NotificationType.Error
            });

            notify.Dispose();
            baseProcess.StandardInput.Close();
            baseProcess.WaitForExit();

            Dispatcher.Invoke(DispatcherPriority.Normal, new Action(async delegate {
                String errorMessage = String.Format(
                    "기반 프로세스가 종료되었습니다.\n\n" +
                    "<Exit time>\n     {0}\n" +
                    "<Exit code>\n     {1:X}\n",
                    baseProcess.ExitTime, baseProcess.ExitCode);
                await this.ShowMessageAsync("오류", errorMessage);
                System.Windows.Application.Current.Shutdown();
            }));
        }

        private async void HelpBtn_Click(object sender, RoutedEventArgs e)
        {
            HelpWindow helpWindow = new HelpWindow();

            this.ShowOverlayAsync();

            bool? dialogResult = helpWindow.ShowDialog();
            switch (dialogResult)
            {
                case true:
                    // User accepted dialog box
                    break;
                case false:
                    // User canceled dialog box
                    break;
                default:
                    // Indeterminate
                    break;
            }
            this.HideOverlayAsync();
        }

        private void RefreshInfo()
        {
            baseProcess.StandardInput.Write("s" + Environment.NewLine);
            baseProcess.StandardInput.Flush();
        }

        private void RefreshBtn_Click(object sender, RoutedEventArgs e)
        {
            RefreshInfo();
        }

        private void InputCheck_Number(object sender, TextCompositionEventArgs e)
        {
            Regex regex = new Regex(@"^[0-9]*$");
            e.Handled = !regex.IsMatch(e.Text);
        }

        private void InputCheck_Decimal(object sender, TextCompositionEventArgs e)
        {
            String newText = (sender as TextBox).Text.Insert((sender as TextBox).CaretIndex, e.Text);
            Regex regex = new Regex(@"^[0-9]([.][0-9]{1,3})?$");
            e.Handled = !regex.IsMatch(newText);
        }
        
        private void ThresholdSlider_Change(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            ThresholdInput.Text = Math.Round(e.NewValue, 3).ToString();
        }

        private void MatchingSlider_Change(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            MatchingInput.Text = Math.Round(e.NewValue, 3).ToString();
        }

        private void OpenSaveDir(object sender, RoutedEventArgs e)
        {
            Process.Start("explorer.exe", imagePath);
        }

        private async void ApplySettings(object sender, RoutedEventArgs e)
        {
            String command =  String.Format("S{0},{1},{2} {3},{4},{5},{6},{7},{8},{9}",
                TimerIntervalInput.Value,
                AlarmIntervalInput.Value,
                SaveIntervalMinInput.Value,
                SaveIntervalMaxInput.Value,
                CornerSizeInput.Value,
                ThresholdInput.Text,
                MatchingInput.Text,
                Convert.ToInt32(DebugCheck.IsChecked),
                Convert.ToInt32(AlarmCheck.IsChecked),
                DeleteIntervalInput.Value
                );

            baseProcess.StandardInput.Write(command + Environment.NewLine);
            baseProcess.StandardInput.Flush();
            this.ShowMessageAsync("변경 사항 적용", "새로운 설정이 적용되었습니다.");
            RefreshInfo();
            Debug.WriteLine(command);
        }
    }
}
