#ifndef INFODISPLAY_H
#define INFODISPLAY_H

// গ্রাফিক্স রিসোর্স (যেমন, টেক্সচার) একবার লোড করার জন্য এই ফাংশনটি কল করা হবে।
void InitializeInfoGraphics();

// প্রতি ফ্রেমে তথ্য (আইকন এবং টেক্সট) রেন্ডার করার জন্য এই ফাংশনটি কল করা হবে।
void RenderInfoDisplay();

#endif // INFODISPLAY_H