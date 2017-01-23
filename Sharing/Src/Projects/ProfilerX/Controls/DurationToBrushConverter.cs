// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;
using System.Windows.Media;

namespace ProfilerX
{
    public class DurationToBrushConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            const float maxValue = 33.3f;   // Set the color to red when we reach 60 fps
            const float halfMaxValue = maxValue / 2f;

            float duration = (float)value;

            float redValue = Math.Max(duration - halfMaxValue, 0f) / halfMaxValue;
            float blueValue = Math.Max(halfMaxValue - duration, 0f) / halfMaxValue;
            float greenValue = 1f - Math.Max(redValue, blueValue);

            return new SolidColorBrush(Color.FromScRgb(1f, redValue, greenValue, blueValue));
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
