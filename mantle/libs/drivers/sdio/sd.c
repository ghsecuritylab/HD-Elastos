/*
 * File      : sd.c
 * 
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author        Notes
 * 2011-07-25     weety     first version
 */

#include <drivers/mmcsd_core.h>
#include <drivers/sd.h>

static const UInt32 tran_unit[] =
{
    10000, 100000, 1000000, 10000000,
    0,     0,      0,       0
};

static const UInt8 tran_value[] =
{
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

static const UInt32 tacc_uint[] =
{
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
};

static const UInt8 tacc_value[] =
{
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

rt_inline UInt32 GET_BITS(UInt32 *resp,
                               UInt32  start,
                               UInt32  size)
{                               
        const Int32 __size = size;
        const UInt32 __mask = (__size < 32 ? 1 << __size : 0) - 1; 
        const Int32 __off = 3 - ((start) / 32);
        const Int32 __shft = (start) & 31;
        UInt32 __res;

        __res = resp[__off] >> __shft;
        if (__size + __shft > 32)
            __res |= resp[__off-1] << ((32 - __shft) % 32);

        return __res & __mask;
}

static Int32 mmcsd_parse_csd(struct rt_mmcsd_card *card)
{
    struct rt_mmcsd_csd *csd = &card->csd;
    UInt32 *resp = card->resp_csd;

    csd->csd_structure = GET_BITS(resp, 126, 2);

    switch (csd->csd_structure)
    {
    case 0:
        csd->taac = GET_BITS(resp, 112, 8);
        csd->nsac = GET_BITS(resp, 104, 8);
        csd->tran_speed = GET_BITS(resp, 96, 8);
        csd->card_cmd_class = GET_BITS(resp, 84, 12);
        csd->rd_blk_len = GET_BITS(resp, 80, 4);
        csd->rd_blk_part = GET_BITS(resp, 79, 1);
        csd->wr_blk_misalign = GET_BITS(resp, 78, 1);
        csd->rd_blk_misalign = GET_BITS(resp, 77, 1);
        csd->dsr_imp = GET_BITS(resp, 76, 1);
        csd->c_size = GET_BITS(resp, 62, 12);
        csd->c_size_mult = GET_BITS(resp, 47, 3);
        csd->r2w_factor = GET_BITS(resp, 26, 3);
        csd->wr_blk_len = GET_BITS(resp, 22, 4);
        csd->wr_blk_partial = GET_BITS(resp, 21, 1);
        csd->csd_crc = GET_BITS(resp, 1, 7);

        card->card_blksize = 1 << csd->rd_blk_len;
        card->card_capacity = (csd->c_size + 1) << (csd->c_size_mult + 2);
        card->card_capacity *= card->card_blksize;
        card->card_capacity >>= 10; /* unit:KB */
        card->tacc_clks = csd->nsac * 100;
        card->tacc_ns = (tacc_uint[csd->taac&0x07] * tacc_value[(csd->taac&0x78)>>3] + 9) / 10;
        card->max_data_rate = tran_unit[csd->tran_speed&0x07] * tran_value[(csd->tran_speed&0x78)>>3];

    #if 0
        val = GET_BITS(resp, 115, 4);
        unit = GET_BITS(resp, 112, 3);
        csd->tacc_ns     = (tacc_uint[unit] * tacc_value[val] + 9) / 10;
        csd->tacc_clks   = GET_BITS(resp, 104, 8) * 100;

        val = GET_BITS(resp, 99, 4);
        unit = GET_BITS(resp, 96, 3);
        csd->max_data_rate    = tran_unit[unit] * tran_value[val];
        csd->ccc      = GET_BITS(resp, 84, 12);

        unit = GET_BITS(resp, 47, 3);
        val = GET_BITS(resp, 62, 12);
        csd->device_size      = (1 + val) << (unit + 2);

        csd->read_bl_len = GET_BITS(resp, 80, 4);
        csd->write_bl_len = GET_BITS(resp, 22, 4);
        csd->r2w_factor = GET_BITS(resp, 26, 3);
    #endif
        break;
    case 1:
        card->flags |= CARD_FLAG_SDHC;

        /*This field is fixed to 0Eh, which indicates 1 ms. 
          The host should not use TAAC, NSAC, and R2W_FACTOR
          to calculate timeout and should uses fixed timeout
          values for read and write operations*/
        csd->taac = GET_BITS(resp, 112, 8);
        csd->nsac = GET_BITS(resp, 104, 8);
        csd->tran_speed = GET_BITS(resp, 96, 8);
        csd->card_cmd_class = GET_BITS(resp, 84, 12);
        csd->rd_blk_len = GET_BITS(resp, 80, 4);
        csd->rd_blk_part = GET_BITS(resp, 79, 1);
        csd->wr_blk_misalign = GET_BITS(resp, 78, 1);
        csd->rd_blk_misalign = GET_BITS(resp, 77, 1);
        csd->dsr_imp = GET_BITS(resp, 76, 1);
        csd->c_size = GET_BITS(resp, 48, 22);

        csd->r2w_factor = GET_BITS(resp, 26, 3);
        csd->wr_blk_len = GET_BITS(resp, 22, 4);
        csd->wr_blk_partial = GET_BITS(resp, 21, 1);
        csd->csd_crc = GET_BITS(resp, 1, 7);

        card->card_blksize = 512;
        card->card_capacity = (csd->c_size + 1) * 512;  /* unit:KB */
        card->tacc_clks = 0;
        card->tacc_ns = 0;
        card->max_data_rate = tran_unit[csd->tran_speed&0x07] * tran_value[(csd->tran_speed&0x78)>>3];

    #if 0
        csd->tacc_ns     = 0;
        csd->tacc_clks   = 0;

        val = GET_BITS(resp, 99, 4);
        unit = GET_BITS(resp, 96, 3);
        csd->max_data_rate    = tran_unit[unit] * tran_value[val];
        csd->ccc      = GET_BITS(resp, 84, 12);

        val = GET_BITS(resp, 48, 22);
        csd->device_size     = (1 + val) << 10;

        csd->read_bl_len = 9;
        csd->write_bl_len = 9;
        /* host should not use this factor and should use 250ms for write timeout */
        csd->r2w_factor = 2;
    #endif
        break;
    default:
        printf("unrecognised CSD structure version %d\n", csd->csd_structure);

        return -RT_ERROR;
    }
    printf("SD card capacity %d KB\n", card->card_capacity);

    return 0;
}

static Int32 mmcsd_parse_scr(struct rt_mmcsd_card *card)
{
    struct rt_sd_scr *scr = &card->scr;
    UInt32 resp[4];

    resp[3] = card->resp_scr[1];
    resp[2] = card->resp_scr[0];
    scr->sd_version = GET_BITS(resp, 56, 4);
    scr->sd_bus_widths = GET_BITS(resp, 48, 4);

    return 0;
}

static Int32 mmcsd_switch(struct rt_mmcsd_card *card)
{
    Int32 err;
    struct rt_mmcsd_host *host = card->host;
    struct rt_mmcsd_req req;
    struct rt_mmcsd_cmd cmd;
    struct rt_mmcsd_data data;
    UInt8 *buf;

    buf = (UInt8*)malloc(64);
    if (!buf) 
    {
        printf("alloc memory failed\n");

        return -RT_ENOMEM;
    }
    
    if (card->card_type != CARD_TYPE_SD)
        goto err;
    if (card->scr.sd_version < SCR_SPEC_VER_1)
        goto err;

    memset(&cmd, 0, sizeof(struct rt_mmcsd_cmd));

    cmd.cmd_code = SD_SWITCH;
    cmd.arg = 0x00FFFFF1;
    cmd.flags = RESP_R1 | CMD_ADTC;

    memset(&data, 0, sizeof(struct rt_mmcsd_data));

    mmcsd_set_data_timeout(&data, card);

    data.blksize = 64;
    data.blks = 1;
    data.flags = DATA_DIR_READ;
    data.buf = (UInt32 *)buf;

    memset(&req, 0, sizeof(struct rt_mmcsd_req));

    req.cmd = &cmd;
    req.data = &data;

    mmcsd_send_request(host, &req);

    if (cmd.err || data.err) 
    {
        goto err1;
    }

    if (buf[13] & 0x02)
        card->hs_max_data_rate = 50000000;

    memset(&cmd, 0, sizeof(struct rt_mmcsd_cmd));

    cmd.cmd_code = SD_SWITCH;
    cmd.arg = 0x80FFFFF1;
    cmd.flags = RESP_R1 | CMD_ADTC;

    memset(&data, 0, sizeof(struct rt_mmcsd_data));

    mmcsd_set_data_timeout(&data, card);

    data.blksize = 64;
    data.blks = 1;
    data.flags = DATA_DIR_READ;
    data.buf = (UInt32 *)buf;

    memset(&req, 0, sizeof(struct rt_mmcsd_req));

    req.cmd = &cmd;
    req.data = &data;

    mmcsd_send_request(host, &req);

    if (cmd.err || data.err) 
    {
        goto err1;
    }

    if ((buf[16] & 0xF) != 1) 
    {
        printf("switching card to high speed failed\n");
        goto err;
    }

    card->flags |= CARD_FLAG_HIGHSPEED;

err:
    rt_free(buf);
    return 0;

err1:
    if (cmd.err)
        err = cmd.err;
    if (data.err)
        err = data.err;

    return err;
}

static Int32 mmcsd_app_cmd(struct rt_mmcsd_host *host,
                              struct rt_mmcsd_card *card)
{
    Int32 err;
    struct rt_mmcsd_cmd cmd = {0};

    cmd.cmd_code = APP_CMD;

    if (card) 
    {
        cmd.arg = card->rca << 16;
        cmd.flags = RESP_R1 | CMD_AC;
    } 
    else 
    {
        cmd.arg = 0;
        cmd.flags = RESP_R1 | CMD_BCR;
    }

    err = mmcsd_send_cmd(host, &cmd, 0);
    if (err)
        return err;

    /* Check that card supported application commands */
    if (!controller_is_spi(host) && !(cmd.resp[0] & R1_APP_CMD))
        return -RT_ERROR;

    return RT_EOK;
}


Int32 mmcsd_send_app_cmd(struct rt_mmcsd_host *host,
                            struct rt_mmcsd_card *card,
                            struct rt_mmcsd_cmd  *cmd,
                            int                   retry)
{
    struct rt_mmcsd_req req;

    UInt32 i; 
    Int32 err;

    err = -RT_ERROR;

    /*
     * We have to resend MMC_APP_CMD for each attempt so
     * we cannot use the retries field in mmc_command.
     */
    for (i = 0;i <= retry;i++) 
    {
        memset(&req, 0, sizeof(struct rt_mmcsd_req));

        err = mmcsd_app_cmd(host, card);
        if (err) 
        {
            /* no point in retrying; no APP commands allowed */
            if (controller_is_spi(host)) 
            {
                if (cmd->resp[0] & R1_SPI_ILLEGAL_COMMAND)
                    break;
            }
            continue;
        }

        memset(&req, 0, sizeof(struct rt_mmcsd_req));

        memset(cmd->resp, 0, sizeof(cmd->resp));

        req.cmd = cmd;
        //cmd->data = NULL;

        mmcsd_send_request(host, &req);

        err = cmd->err;
        if (!cmd->err)
            break;

        /* no point in retrying illegal APP commands */
        if (controller_is_spi(host)) 
        {
            if (cmd->resp[0] & R1_SPI_ILLEGAL_COMMAND)
                break;
        }
    }

    return err;
}

Int32 mmcsd_app_set_bus_width(struct rt_mmcsd_card *card, Int32 width)
{
    Int32 err;
    struct rt_mmcsd_cmd cmd;

    memset(&cmd, 0, sizeof(struct rt_mmcsd_cmd));

    cmd.cmd_code = SD_APP_SET_BUS_WIDTH;
    cmd.flags = RESP_R1 | CMD_AC;

    switch (width) 
    {
    case MMCSD_BUS_WIDTH_1:
        cmd.arg = MMCSD_BUS_WIDTH_1;
        break;
    case MMCSD_BUS_WIDTH_4:
        cmd.arg = MMCSD_BUS_WIDTH_4;
        break;
    default:
        return -RT_ERROR;
    }

    err = mmcsd_send_app_cmd(card->host, card, &cmd, 3);
    if (err)
        return err;

    return RT_EOK;
}

Int32 mmcsd_send_app_op_cond(struct rt_mmcsd_host *host,
                                UInt32           ocr,
                                UInt32          *rocr)
{
    struct rt_mmcsd_cmd cmd;
    UInt32 i;
    Int32 err = RT_EOK;

    memset(&cmd, 0, sizeof(struct rt_mmcsd_cmd));

    cmd.cmd_code = SD_APP_OP_COND;
    if (controller_is_spi(host))
        cmd.arg = ocr & (1 << 30); /* SPI only defines one bit */
    else
        cmd.arg = ocr;
    cmd.flags = RESP_SPI_R1 | RESP_R3 | CMD_BCR;

    for (i = 100; i; i--) 
    {
        err = mmcsd_send_app_cmd(host, NULL, &cmd, 3);
        if (err)
            break;

        /* if we're just probing, do a single pass */
        if (ocr == 0)
            break;

        /* otherwise wait until reset completes */
        if (controller_is_spi(host)) 
        {
            if (!(cmd.resp[0] & R1_SPI_IDLE))
                break;
        } 
        else 
        {
            if (cmd.resp[0] & CARD_BUSY)
                break;
        }

        err = -RT_ETIMEOUT;

        mmcsd_delay_ms(10); //delay 10ms
    }

    if (rocr && !controller_is_spi(host))
        *rocr = cmd.resp[0];

    return err;
}

/*
 * To support SD 2.0 cards, we must always invoke SD_SEND_IF_COND
 * before SD_APP_OP_COND. This command will harmlessly fail for
 * SD 1.0 cards.
 */
Int32 mmcsd_send_if_cond(struct rt_mmcsd_host *host, UInt32 ocr)
{
    struct rt_mmcsd_cmd cmd;
    Int32 err;
    UInt8 pattern;

    cmd.cmd_code = SD_SEND_IF_COND;
    cmd.arg = ((ocr & 0xFF8000) != 0) << 8 | 0xAA;
    cmd.flags = RESP_SPI_R7 | RESP_R7 | CMD_BCR;

    err = mmcsd_send_cmd(host, &cmd, 0);
    if (err)
        return err;

    if (controller_is_spi(host))
        pattern = cmd.resp[1] & 0xFF;
    else
        pattern = cmd.resp[0] & 0xFF;

    if (pattern != 0xAA)
        return -RT_ERROR;

    return RT_EOK;
}

Int32 mmcsd_get_card_addr(struct rt_mmcsd_host *host, UInt32 *rca)
{
    Int32 err;
    struct rt_mmcsd_cmd cmd;

    memset(&cmd, 0, sizeof(struct rt_mmcsd_cmd));

    cmd.cmd_code = SD_SEND_RELATIVE_ADDR;
    cmd.arg = 0;
    cmd.flags = RESP_R6 | CMD_BCR;

    err = mmcsd_send_cmd(host, &cmd, 3);
    if (err)
        return err;

    *rca = cmd.resp[0] >> 16;

    return RT_EOK;
}

#define be32_to_cpu(x) ((UInt32)(              \
    (((UInt32)(x) & (UInt32)0x000000ffUL) << 24) |        \
    (((UInt32)(x) & (UInt32)0x0000ff00UL) <<  8) |        \
    (((UInt32)(x) & (UInt32)0x00ff0000UL) >>  8) |        \
    (((UInt32)(x) & (UInt32)0xff000000UL) >> 24)))

Int32 mmcsd_get_scr(struct rt_mmcsd_card *card, UInt32 *scr)
{
    Int32 err;
    struct rt_mmcsd_req req;
    struct rt_mmcsd_cmd cmd;
    struct rt_mmcsd_data data;

    err = mmcsd_app_cmd(card->host, card);
    if (err)
        return err;

    memset(&req, 0, sizeof(struct rt_mmcsd_req));
    memset(&cmd, 0, sizeof(struct rt_mmcsd_cmd));
    memset(&data, 0, sizeof(struct rt_mmcsd_data));

    req.cmd = &cmd;
    req.data = &data;

    cmd.cmd_code = SD_APP_SEND_SCR;
    cmd.arg = 0;
    cmd.flags = RESP_SPI_R1 | RESP_R1 | CMD_ADTC;

    data.blksize = 8;
    data.blks = 1;
    data.flags = DATA_DIR_READ;
    data.buf = scr;

    mmcsd_set_data_timeout(&data, card);

    mmcsd_send_request(card->host, &req);

    if (cmd.err)
        return cmd.err;
    if (data.err)
        return data.err;

    scr[0] = be32_to_cpu(scr[0]);
    scr[1] = be32_to_cpu(scr[1]);

    return 0;
}


static Int32 mmcsd_sd_init_card(struct rt_mmcsd_host *host,
                                     UInt32           ocr)
{
    struct rt_mmcsd_card *card;
    Int32 err;
    UInt32 resp[4];
    UInt32 max_data_rate;

    mmcsd_go_idle(host);

    /*
     * If SD_SEND_IF_COND indicates an SD 2.0
     * compliant card and we should set bit 30
     * of the ocr to indicate that we can handle
     * block-addressed SDHC cards.
     */
    err = mmcsd_send_if_cond(host, ocr);
    if (!err)
        ocr |= 1 << 30;

    err = mmcsd_send_app_op_cond(host, ocr, NULL);
    if (err)
        goto err;

    if (controller_is_spi(host))
        err = mmcsd_get_cid(host, resp);
    else
        err = mmcsd_all_get_cid(host, resp);
    if (err)
        goto err;

    card = malloc(sizeof(struct rt_mmcsd_card));
    if (!card) 
    {
        printf("malloc card failed\n");
        err = -RT_ENOMEM;
        goto err;
    }
    memset(card, 0, sizeof(struct rt_mmcsd_card));

    card->card_type = CARD_TYPE_SD;
    card->host = host;
    memcpy(card->resp_cid, resp, sizeof(card->resp_cid));

    /*
     * For native busses:  get card RCA and quit open drain mode.
     */
    if (!controller_is_spi(host)) 
    {
        err = mmcsd_get_card_addr(host, &card->rca);
        if (err)
            goto err1;

        mmcsd_set_bus_mode(host, MMCSD_BUSMODE_PUSHPULL);
    }

    err = mmcsd_get_csd(card, card->resp_csd);
    if (err)
        goto err1;

    err = mmcsd_parse_csd(card);
    if (err)
        goto err1;

    if (!controller_is_spi(host)) 
    {
        err = mmcsd_select_card(card);
        if (err)
            goto err1;
    }

    err = mmcsd_get_scr(card, card->resp_scr);
    if (err)
        goto err1;

    mmcsd_parse_scr(card);

    if (controller_is_spi(host)) 
    {
        err = mmcsd_spi_use_crc(host, 1);
        if (err)
            goto err1;
    }

    /*
     * change SD card to high-speed, only SD2.0 spec
     */
    err = mmcsd_switch(card);
    if (err)
        goto err1;

    /* set bus speed */
    max_data_rate = (unsigned int)-1;

    if (card->flags & CARD_FLAG_HIGHSPEED) 
    {
        if (max_data_rate > card->hs_max_data_rate)
            max_data_rate = card->hs_max_data_rate;
    } 
    else if (max_data_rate > card->max_data_rate) 
    {
        max_data_rate = card->max_data_rate;
    }

    mmcsd_set_clock(host, max_data_rate);

    /*switch bus width*/
    if ((host->flags & MMCSD_BUSWIDTH_4) &&
        (card->scr.sd_bus_widths & SD_SCR_BUS_WIDTH_4)) 
    {
        err = mmcsd_app_set_bus_width(card, MMCSD_BUS_WIDTH_4);
        if (err)
            goto err1;

        mmcsd_set_bus_width(host, MMCSD_BUS_WIDTH_4);
    }

    host->card = card;

    return 0;

err1:
    rt_free(card);
err:

    return err;
}

/*
 * Starting point for SD card init.
 */
Int32 init_sd(struct rt_mmcsd_host *host, UInt32 ocr)
{
    Int32 err;
    UInt32  current_ocr;
    /*
     * We need to get OCR a different way for SPI.
     */
    if (controller_is_spi(host))
    {
        mmcsd_go_idle(host);

        err = mmcsd_spi_read_ocr(host, 0, &ocr);
        if (err)
            goto err;
    }

    if (ocr & VDD_165_195)
    {
        printf(" SD card claims to support the "
               "incompletely defined 'low voltage range'. This "
               "will be ignored.\n");
        ocr &= ~VDD_165_195;
    }

    current_ocr = mmcsd_select_voltage(host, ocr);

    /*
     * Can we support the voltage(s) of the card(s)?
     */
    if (!current_ocr)
    {
        err = -RT_ERROR;
        goto err;
    }

    /*
     * Detect and init the card.
     */
    err = mmcsd_sd_init_card(host, current_ocr);
    if (err)
        goto err;

    mmcsd_host_unlock(host);

    err = rt_mmcsd_blk_probe(host->card);
    if (err)
        goto remove_card;
    mmcsd_host_lock(host);

    return 0;

remove_card:
    mmcsd_host_lock(host);
    rt_mmcsd_blk_remove(host->card);
    rt_free(host->card);
    host->card = NULL;
err:

    printf("init SD card failed\n");

    return err;
}
